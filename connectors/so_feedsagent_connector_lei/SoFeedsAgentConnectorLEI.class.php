<?php

class SoFeedsAgentConnectorLEI extends SoFeedsAgentConnectorAbstract
{
    public static function getFeatures() {

        return array(
            'implements' => array(
                'generator',
                'importer',
            ),
            'use_multiple_fields' => true,// TODO : obligatoire - supprimer tout ce qui se rapport à ce paramètre (code, CSS, etc.)
            'use_fields_compatibility' => true, // TODO : obligatoire - supprimer tout ce qui se rapport à ce paramètre (code, CSS, etc.)
        );
    }

    public function connectorConfigurationForm(array &$form, array &$form_state, $touch_form = false) {

        if($touch_form == true) {
            return true;
        }

        $form = array(
            '#type' => 'vertical_tabs',
            '#title' => "LEI",
            '#theme_wrappers' => array('vertical_tabs', 'fieldset'),
        );

        $form['connection'] = array(
            '#type' => 'fieldset',
            '#title' => t("Connection"),

            'base_url' => array(
                '#type' => 'textfield',
                '#description' => t("Without trailing slash."),
                '#title' => t("Base URL"),
                '#default_value' => $this->_definition['connection']['base_url'],
                '#required' => true,
            ),

            'user' => array(
                '#type' => 'textfield',
                '#title' => t("User"),
                '#default_value' => $this->_definition['connection']['user'],
                '#required' => true,
                '#size' => 10,
            ),

            'pwkey' => array(
                '#type' => 'textfield',
                '#title' => t("Key"),
                '#default_value' => $this->_definition['connection']['pwkey'],
                '#required' => true,
                '#size' => 45,
            ),
        );

        $form['schema'] = array(
            '#type' => 'fieldset',
            '#title' => t("Schema"),
        );

        $languages = language_list();
        $default_language = language_default('language');

        foreach($languages as $language) {

            $type = $language->language != 'fr' ? 'textfield' : 'item';

            $form['schema'][$language->language] = array(
                '#type' => $type,
                '#title' => t($language->name) . ($language->language == $default_language ? " (" . t("default") . ")" : ""),
                '#default_value' => !empty($this->_definition['schema'][$language->language]) ? $this->_definition['schema'][$language->language] : strtoupper($language->language),
                '#field_prefix' => 'WEBACCESS' . ($language->language != 'fr' ? '_' : ''),
                '#size' => 8,
            );
        }

        $form['listing'] = array(
            '#type' => 'fieldset',
            '#title' => t("Listing"),

            'script' => array(
                '#type' => 'textfield',
                '#title' => t("Script"),
                '#default_value' => !empty($this->_definition['listing']['script']) ? $this->_definition['listing']['script'] : "listeproduits.asp",
                '#required' => true,
                '#size' => 40,
            ),

            'url_params' => array(
                '#type' => 'textfield',
                '#title' => t("URL params"),
                '#description' => t("Raw format (with &amp;)"),
                '#default_value' => !empty($this->_definition['listing']['url_params']) ? $this->_definition['listing']['url_params'] : "&urlnames=tous",
                '#size' => 40,
                '#maxlength' => 2048,
            ),

            'listing_max_length' => array(
                '#type' => 'textfield',
                '#title' => t("Listing section max length"),
                '#description' => t("0 = unlimited"),
                '#field_suffix' => "produits",
                '#default_value' => !empty($this->_definition['listing']['listing_max_length']) ? $this->_definition['listing']['listing_max_length'] : 500,
                '#size' => 4,
                '#element_validate' => array('element_validate_integer_positive'),
            ),
        );

        $form['fiche'] = array(
            '#type' => 'fieldset',
            '#title' => t("Fiche"),

            'script' => array(
                '#type' => 'textfield',
                '#title' => t("Script"),
                '#default_value' => !empty($this->_definition['fiche']['script']) ? $this->_definition['fiche']['script'] : "ficheproduit.asp",
                '#required' => true,
                '#size' => 40,
            ),

            'url_params' => array(
                '#type' => 'textfield',
                '#title' => t("URL params"),
                '#description' => t("Raw format (with &amp;)"),
                '#default_value' => !empty($this->_definition['fiche']['url_params']) ? $this->_definition['fiche']['url_params'] : "&urlnames=tous&lxml=sit_fiche",
                '#size' => 40,
                '#maxlength' => 2048,
            ),
        );
    }

    public function generatorListingParamsStep(array &$step_elements, array &$form_state, array $args = array()) {

        // attention : la structure de wizzard_params['params'] est différente lors d'une création : elle contient l'item et le feed

        if(empty($form_state['values']['step_elements'])) {
            if(!empty($form_state['wizzard_params']['params']['connector'])) {
                // type virtuel déjà paramétré par le passé
                if(!array_key_exists('feed', $form_state['wizzard_params']['params']['connector'])) {

                    $produit_default = $form_state['wizzard_params']['params']['connector']['produit'];
                    $use_input_mask_default = $form_state['wizzard_params']['params']['connector']['use_input_mask'];
                    $input_mask_default = $form_state['wizzard_params']['params']['connector']['input_mask'];
                    $nomenclature_default = $form_state['wizzard_params']['params']['connector']['nomenclature'];
                    $extractions_default = $form_state['wizzard_params']['params']['connector']['extractions'];

                // type virtuel créé à la volée par l'importer
                } elseif(array_key_exists('feed', $form_state['wizzard_params']['params']['connector'])) {

                    $produit_default = $form_state['wizzard_params']['params']['connector']['item']['id'];
                    $use_input_mask_default = false;
                    $input_mask_default = "";
                    $nomenclature_default = array('32');
                    $extractions_default = array();

                }

            // pas de type virtuel sélectionné
            } else {
                $produit_default = "";
                $use_input_mask_default = false;
                $input_mask_default = "";
                $nomenclature_default = array('32');
                $extractions_default = array();
            }

        } else {
            $produit_default = array_key_exists('produit', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['produit'] : "";
            $use_input_mask_default = array_key_exists('use_input_mask', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['use_input_mask'] : 0;
            $input_mask_default = array_key_exists('input_mask', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['input_mask'] : "";
            $nomenclature_default = array_key_exists('nomenclature', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['nomenclature'] : array('32');
            $extractions_default = array_key_exists('extractions', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['extractions'] : array();
        }

        $step_elements['produit'] = array(
            '#type' => 'textfield',
            '#title' => t("Produit ID"),
            '#description' => t("To use as reference"),
            '#default_value' => $produit_default,
            '#size' => 12,
            '#required' => true,
        );

        $step_elements['use_input_mask'] = array(
            '#type' => 'checkbox',
            '#title' => t("Use an input mask"),
            '#default_value' => $use_input_mask_default,
        );

        $step_elements['input_mask'] = array(
            '#type' => 'textfield',
            '#title' => t("Input mask"),
            '#description' => t("URL starting with : 'http://'.<br />Format : listeproduits.asp?rfrom=1&rto=1&user=<em>[user]</em>&pwkey=<em>[key]</em>&urlnames=tous&lepool=<em>[pool]</em>"),
            '#default_value' => $input_mask_default,
            '#maxlength' => 512,
            '#states' => array(
                'visible' => array(
                    ':input[name="step_elements[use_input_mask]"]' => array('checked' => true),
                ),
            ),
        );

        $step_elements['nomenclature'] = array(
            '#type' => 'checkboxes',
            '#title' => t("Content type fields"),
            '#options' => array(
                '32' => t("Include whole criteria definitions"), // 32 : Critères de type (bordereau)
                '4' => t("Include prestataire informations"),
            ),
            '#default_value' => !empty($nomenclature_default) ? $nomenclature_default : array('32'),
        );

        $step_elements['extractions'] = array(
            '#type' => 'checkboxes',
            '#title' => t("Create fields for these additional features"),
            '#options' => array(
                'schedule' => t("Schedule"),
                //'8' => t("Availabilities"),
            ),
            '#default_value' => $extractions_default,
        );
    }

    public function generatorListingParamsStepSubmission(array &$form_state, $obsolescence) {

        _so_feedsagent_register_dependency($form_state, 'produit');
        _so_feedsagent_register_dependency($form_state, 'input_mask');
        _so_feedsagent_register_dependency($form_state, 'nomenclature');
        _so_feedsagent_register_dependency($form_state, 'extractions');
        _so_feedsagent_register_dependency($form_state, 'feed_fields', array('connector', 'produit', 'nomenclature', 'extractions'));

        // si les paramètres propre au connecteur n'ont pas encore été définis, ou que ces paramètres ont changé
        if(empty($form_state['wizzard_params']['params']['connector']) || $obsolescence == true) {

            $form_state['wizzard_params']['params']['connector'] = $form_state['values']['step_elements'];
            _so_feedsagent_invalidate_dependencies($form_state, array('produit', 'input_mask', 'nomenclature', 'extractions'));
        }

        // si la liste n'existe pas,...
        if(!array_key_exists('feed_fields', $form_state['wizzard_params'])
                // ... ou si la liste a été invalidée,...
                || _so_feedsagent_get_dependency_state($form_state, 'feed_fields') == true) {

            $this->generatorPopulateFieldsDefinitionsList($form_state['wizzard_params']);
            _so_feedsagent_invalidate_dependencies($form_state, array('feed_fields'));

            $content_types_definition = so_feedsagent_get_content_types_definitions(null, $this->_definition['id'], $form_state['wizzard_params']['primary_type'], true);

            if(!empty($content_types_definition)) {

                $content_types_definition = array_pop($content_types_definition);

                // si l'on n'est pas dans le cadre d'une édition
                if(empty($form_state['wizzard_params']['id'])) {
                    // déjà mappé sur un autre type Drupal
                    if(!empty($content_types_definition->type) && $content_types_definition->type != $form_state['wizzard_params']['type']) {

                        drupal_set_message(
                            t(
                                "A LEI virtual type named '@name' and matching the primary type '@primary' already exists.<br />It's mapped on '@type' Drupal's content type.",
                                array(
                                    '@name' => $content_types_definition->virtual_name,
                                    '@primary' => $content_types_definition->primary_type,
                                    '@type' => $content_types_definition->type,
                                )
                            ),
                            'error'
                        );

                        return false;

                    // virtual type orphelin : on peut donc l'utiliser
                    } elseif(empty($form_state['wizzard_params']['id'])) {
                        _so_feedsagent_register_dependency($form_state, 'id', array('connector', 'produit'));
                        _so_feedsagent_invalidate_dependencies($form_state, array('id'));
                        $form_state['wizzard_params']['id'] = $content_types_definition->id;

                        drupal_set_message(
                            t(
                                "A LEI virtual type named '@name' and matching the primary type '@primary' already exists.<br />Since it's not mapped on any Drupal's content type, it can be redefined.",
                                array(
                                    '@name' => $content_types_definition->virtual_name,
                                    '@primary' => $content_types_definition->primary_type,
                                    '@type' => $content_types_definition->type,
                                )
                            ),
                            'warning'
                        );
                    }
                }
            }
        }

        _so_feedsagent_reset_dependency_state($form_state, 'feed_fields');

        $form_state['values']['step_elements'] = array();
    }

    public function generatorMappingParamsStep(&$step_elements, &$form_state, $args = array()) {
        so_feedsagent_generator_virtual_type_params_step($step_elements, $form_state, $args);
    }

    public function generatorMappingParamsStepSubmission(&$form_state, $obsolescence) {

        if($obsolescence == true) {
            // on se contente juste de loader les données
            if(!empty($form_state['values']['step_elements']['primary_type'])) {
                $definition = so_feedsagent_get_content_types_definitions(null, $this->_definition['id'], $form_state['values']['step_elements']['primary_type'], true);
                $type_definition = (array)array_pop($definition);
                $form_state['wizzard_params'] = array_merge($form_state['wizzard_params'], $type_definition);

                _so_feedsagent_invalidate_dependencies($form_state, array('id', 'fields', 'groups', 'params')); // 'type' et 'primary_type' ont déjà été invalidés
            }
        }

        // on laisse le soin à la mécanique d'origine de stocker les valeurs
        so_feedsagent_generator_virtual_type_params_step_submission($form_state, $obsolescence);
    }

    public function generatorDuplicateParamsStep(&$step_elements, &$form_state, $args = array()) {

        $types = so_feedsagent_get_content_types_definitions(null, $this->_definition['id'], null, true);

        $options = array();

        foreach($types as $type) {
            if(!empty($type->type)) {continue;}

            $options[$type->id] = $type->virtual_name;
        }

        $step_elements = array(
            '#type' => 'container',

            'target_types' => array(
                '#type' => 'checkboxes',
                '#title' => t("Map to virtual type(s)"),
                '#options' => $options,
            ),
        );
    }

    public function generatorDuplicateParamsStepSubmission(&$form_state, $obsolescence) {

        $targets = array_values(array_filter($form_state['values']['step_elements']['target_types']));

        $record_source = (array)so_feedsagent_get_content_types_definitions($form_state['wizzard_params']['id']);

        $form_state['wizzard_params']['#lei_data'] = array();

        foreach($targets as $id) {
            $record = (array)so_feedsagent_get_content_types_definitions($id);

            $record['type'] = $record_source['type'];
            $record['fields'] = serialize($record_source['fields']);
            $record['groups'] = serialize($record_source['groups']);

            $produit = $record['params']['connector']['item']['id'];

            $record['params'] = $record_source['params'];
            $record['params']['connector']['produit'] = $produit;

            $record['params'] = serialize($record['params']);

            unset($record['name']);// champ JOINED ajouté par so_feedsagent_generator_content_type_params_step();

            drupal_write_record('feedsagent_content_types', $record, 'id');

            $form_state['wizzard_params']['#lei_data'][] = $record['virtual_name'];
        }

        $form_state['values']['step_elements'] = array();
    }

    public function generatorDuplicateParamsEndStep(&$step_elements, &$form_state, $args = array()) {

        $step_elements = array(
            '#markup' => format_plural(
                count($form_state['wizzard_params']['#lei_data']),
                "1 virtual type has been set",
                "@count virtual types have been set"
            ) . " : " . "<ul><li>" . implode('</li><li>', $form_state['wizzard_params']['#lei_data']) . "</li></ul>",
        );
    }

    public function generatorPopulateFieldsDefinitionsList(array &$wizzard_params) {

        // 1 : on prends aussi les critères de la fiche, pour l'affichage des valeurs informatives.
        $url = $this->_getFicheURL($wizzard_params['params']['connector']['produit'], language_default('name'), 1 + array_sum($wizzard_params['params']['connector']['nomenclature']));

        $xml = simplexml_load_file($url, null, LIBXML_NOCDATA);

        // DEV : tester la validité du XML et transmettre les éventuelles infos d'erreur
        $data = $this->_xml2array($xml->Resultat->sit_fiche);

        // les champs système...
        $fields_list = array(
            // ...que l'on rassemble au sein d'un groupe portant le nom du connecteur
            $this->_definition['id'] => array(
                'label' => $this->_definition['label'],
                'type' => 'group',
                'locked' => true,
            ),
            'PRODUIT' => array(
                'label' => t("Produit"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['PRODUIT']),
                'locked' => true,
            ),
            'TYPE_NOM' => array(
                'label' => t("Produit type"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['TYPE_NOM']),
                'locked' => true,
            ),
            'DATMAJ' => array(
                'label' => t("Updated"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['DATMAJ']),
                'locked' => true,
            ),
        );

        $wizzard_params['primary_type'] = $data['TYPE_DE_PRODUIT'] . '@' . $this->_definition['id'];
        $wizzard_params['virtual_name'] = $data['TYPE_NOM'];

        // les horaires
        if(!empty($wizzard_params['params']['connector']['extractions']['schedule'])) {

            $fields_list += array(
                'schedule' => array(
                    'label' => t("Schedule"),
                    'type' => 'group',
                    'prevent_sorting' => true,
                ),

                'schedule_dates' => array(
                    'label' => t("Dates"),
                    'type' => 'date',
                    'multiple' => true,
                    'group' => 'schedule',
                ),
                /* // TODO : les heures sont multiples et sont associées aux dates, qui, elles-aussi sont multiples
                'schedule_hours' => array(
                    'label' => t("Hours"),
                    'type' => 'textfield',
                    'multiple' => true,
                    'group' => 'schedule',
                    'locked' => true,
                ),

                'schedule_comment' => array( // les commentaires sont également associés aux dates (ils sont unique pour chaque date)
                    'label' => t("Comment"),
                    'type' => 'textarea',
                    'group' => 'schedule',
                    'locked' => true,
                ),
                */
            );

        }

        unset($data['TYPE_DE_PRODUIT']);
        unset($data['PRODUIT']);
        unset($data['DATMAJ']);

        // pas de array_merge ! les indexes pseudo-numériques du LEI seraient considérés comme des int, et les array serait alors réindexés
        // de plus, on s'est assuré que les champs déjà définis soient supprimés de $data
        $fields_list += $this->_getFieldsDefinitionFromData($data);

        $wizzard_params['feed_fields'] = $fields_list;

        // masque de saisie
        if($wizzard_params['params']['connector']['use_input_mask'] == true && !empty($wizzard_params['params']['connector']['input_mask'])) {

            $wizzard_params['custom_selection_label'] = t("Input mask");

            $mask_url = $wizzard_params['params']['connector']['input_mask'];

            $timeout = 120;
            drupal_set_time_limit($timeout);

            $fp = fopen($mask_url, 'r', false, stream_context_create(array(
                'http' => array(
                    'method'=>"GET",
                    'timeout' => $timeout,
                ),
            )));

            $xml_string = stream_get_contents($fp);
            $xml = simplexml_load_string($xml_string);

            $data = $this->_xml2array($xml->NOMENCLATURE);

            foreach($data['CRIT'] as $critere) {

                $wizzard_params['custom_selection'][$critere['attributes']['CLEF']] = 1;

                if($critere['attributes']['QUAL'] == 2) {
                    foreach($critere['MODAL'] as $key => $val) {
                        $wizzard_params['custom_selection'][$key . '__' . $critere['attributes']['CLEF']] = 1;
                    }
                }
            }

        } else {
            $wizzard_params['custom_selection'] = array();
        }
    }

    /*
     * We wrappe the native function, because we need our own submit method (automatically derived from the wrapper)
     */
    public function generatorEditVirtualParamsStep(&$step_elements, &$form_state, $args = array()) {
        so_feedsagent_generator_virtual_type_params_step($step_elements, $form_state, $args);
    }

    public function generatorEditVirtualParamsStepSubmission(&$form_state, $obsolescence) {

        if($obsolescence == true) {
            // on se contente juste de loader les données
            if(!empty($form_state['values']['step_elements']['primary_type'])) {
                $definition = so_feedsagent_get_content_types_definitions(null, $this->_definition['id'], $form_state['values']['step_elements']['primary_type'], true);
                $definition = (array)array_pop($definition);
                $form_state['wizzard_params'] = array_merge($form_state['wizzard_params'], $definition);

                _so_feedsagent_invalidate_dependencies($form_state, array('id', 'fields', 'groups', 'params')); // 'type' et 'primary_type' ont déjà été invalidés
            }
        }

        // on laisse le soin à la mécanique d'origine de stocker les valeurs
        so_feedsagent_generator_virtual_type_params_step_submission($form_state, $obsolescence);
    }

    public function importerFeedConfigurationForm(array &$form, array $configuration) {

        $form['#title'] = "LEI";

        $form['clause'] = array(
            '#type' => 'textfield',
            '#title' => "Clause",
            '#size' => 16,
            '#required' => true,
            '#default_value' => $configuration['clause'],
        );

        $form['url_params'] = array(
            '#type' => 'textfield',
            '#title' => t("URL clause params"),
            '#description' => t("'PVALUES', 'PNAMES', 'leshoraires', etc.<br />Raw format (with &amp;)"),
            '#default_value' => $configuration['url_params'],
            '#size' => 100,
            '#maxlength' => 2048,
        );

        $form['couplages'] = array(
            '#type' => 'textfield',
            '#title' => t("Couplages code for \"is parent of\""),
            '#size' => 2,
            '#default_value' => $configuration['couplages'],
        );
    }

    public function importerDisplayFeedInfos(array $configuration) {

        if(empty($configuration)) {return;}

        $output = "";

        drupal_add_js(drupal_get_path('module', 'so_feedsagent_connector_lei') . '/scripts/so_feedsagent_connector_lei.js');
        drupal_add_css(drupal_get_path('module', 'so_feedsagent_connector_lei') . '/css/so_feedsagent_connector_lei.css');

        $url_params = explode('&', $configuration['url_params']);
        $url_params = array_filter($url_params);

        $url_params_list = array(
            '#theme' => 'item_list',
            '#type' => 'ul',
            '#items' => $url_params,
        );

        $output .= "<strong>Clause :</strong> " . $configuration['clause'] . "<br />";

        if(!empty($url_params_list['#items'])) {
            $output .= "<strong>" . t("Parameters") . " :</strong> " . render($url_params_list);
        }

        $see_xml = array(
            '#type' => 'container',
            '#attributes' => array(
                'class' => array('connector_lei_see_xml', 'clearfix')
            ),

            'rfrom' => array(
                '#type' => 'textfield',
                '#title' => "rfrom",
                '#size' => 3,
                '#value' => 1,
                '#attributes' => array(
                    'class' => array('connector_lei_see_xml_rfrom')
                ),
            ),
            'rto' => array(
                '#type' => 'textfield',
                '#title' => "rto",
                '#size' => 3,
                '#value' => 1500,
                '#attributes' => array(
                    'class' => array('connector_lei_see_xml_rto')
                ),
            ),

            'link' => array(
                '#markup' => l(
                    t("See XML"),
                    $this->_getListingURL($configuration['clause'], $configuration['url_params'], !empty($configuration['couplages']), null),
                    array(
                        'attributes' => array(
                            'target' => '_blank',
                            'class' => array('connector_lei_see_xml_link'),
                        ),
                    )
                ),
            ),
        );

        $output .= render($see_xml);

        return $output;
    }

    public function importerGetFeedListing(array $configuration) {

        $base_url = $this->_getListingURL($configuration['clause'], $configuration['url_params'], !empty($configuration['couplages']), null);
        $rfrom = 1;
        $remaining = $this->_definition['listing']['listing_max_length'];
        $data = array();
        $timeout = 123; // 12'3' pour repérage dans les logs en cas de problème.

        while($remaining > 0) {
            $rto = $rfrom + $this->_definition['listing']['listing_max_length'] - 1;

            $url = $base_url . "&rfrom=" . $rfrom . "&rto=" . $rto;

            $fp = fopen($url, 'r', false, stream_context_create(array(
                'http' => array(
                    'method'=>"GET",
                    'timeout' => $timeout,
                ),
            )));

            if($fp) {
                $xml_string = stream_get_contents($fp);

                if(!empty($xml_string)) {
                    $xml = simplexml_load_string($xml_string);
                } else {
                    return t("Server didn't return any XML data (rfom=@rfrom, rto=@rto)", array('@rfrom' => $rfrom, '@rto' => $rto));
                }
            } else {
                return t("Server didn't respond (rfom=@rfrom, rto=@rto)", array('@rfrom' => $rfrom, '@rto' => $rto));
            }

            fclose($fp);

            if(empty($xml->Resultat)) {
                return t("XML was empty (rfom=@rfrom, rto=@rto)", array('@rfrom' => $rfrom, '@rto' => $rto));
            }

            if($rfrom == 1) {
                $remaining = (int)$xml->Resultat[0]['TOTAL_FENETRE'];
            }

            $remaining -= $this->_definition['listing']['listing_max_length'];
            $rfrom += $this->_definition['listing']['listing_max_length'];

            $tmp_data = $this->_xml2array($xml->Resultat);
            $data = array_merge($data, $tmp_data['sit_liste']);
        }

        $listing = array();

        $missing = array();

        if(!array_key_exists('PRODUIT', $data[0])) {
            $missing[] = "PRODUIT";
        }

        if(!array_key_exists('TYPE_DE_PRODUIT', $data[0])) {
            $missing[] = "TYPE_DE_PRODUIT";
        }

        if(!array_key_exists('TYPE_NOM', $data[0])) {
            $missing[] = "TYPE_NOM";
        }

        if(!array_key_exists('DATMAJ', $data[0])) {
            $missing[] = "DATMAJ";
        }

        if(!empty($missing)) {
            return t("Feed is invalid. One or more parameters missing : @missing", array('@missing' => implode(', ', $missing)));
        }

        $parent_products = array();

        foreach($data as $weight => $fiche) {

            $datemaj = DateTime::createFromFormat('d/m/Y H:i:s', $fiche['DATMAJ']);

            $listing[$fiche['PRODUIT']] = array(
                'id' => $fiche['PRODUIT'],
                'parent_id' => null,
                'primary_type' => $fiche['TYPE_DE_PRODUIT'] . '@' . $this->_definition['id'],
                'primary_type_name' => $fiche['TYPE_NOM'],
                'date' => $datemaj->format('U'),
                'weight' => $weight,
            );

            if(array_key_exists('Produits_associes', $fiche)) {

                foreach($fiche['Produits_associes'] as $related_product) {

                    if($related_product['TYPE_DE_COUPLAGE'] == $configuration['couplages']) {
                        $parent_products[$fiche['PRODUIT']] = $related_product['PRODUIT_ASSOCIE'];
                    }
                }
            }
        }

        foreach($listing as $id => $item) {
            if(array_key_exists($item['id'], $parent_products) && array_key_exists($parent_products[$id], $listing)) {
                $listing[$id]['parent_id'] = $parent_products[$id];
            }
        }

        return $listing;
    }

    public function importerGetItemURL($item_id, $language, stdClass $type_definition, stdClass $feed_definition) {

        $configuration = $type_definition->params['connector'];

        $nomenclature = 1 + array_sum($configuration['nomenclature']); // 1 : criteres
        $nomenclature = $nomenclature >= 32 ? $nomenclature - 32 : $nomenclature; // on ne veut surtout pas les critères de type
        $nomenclature += in_array('schedule', $configuration['extractions'], true) ? 2 : 0; // 2 : les horaires

        return $this->_getFicheURL($item_id, $language, $nomenclature);
    }

    public function importerGetFeedValues($node, $item_id, array &$title, $language, array $configuration, array &$fields, stdClass $feed_definition) {

        $nomenclature = 1 + array_sum($configuration['nomenclature']); // 1 : criteres
        $nomenclature = $nomenclature >= 32 ? $nomenclature - 32 : $nomenclature; // on ne veut surtout pas les critères de type
        $nomenclature += in_array('schedule', $configuration['extractions'], true) ? 2 : 0; // 2 : les horaires

        $url = $this->_getFicheURL($item_id, $language, $nomenclature);

        $xml = simplexml_load_file($url, null, LIBXML_NOCDATA);

        if(empty($xml)) {
            return t("Server didn't provide any XML data");
        } elseif(empty($xml->Resultat)) {
            return t("XML was empty");
        }

        $data = $this->_xml2array($xml->Resultat);

        $this->_getFieldsValuesFromData($data['sit_fiche'][0], $fields, $title, $language, $node);

        return;
    }

    private function _getListingURL($clause, $params, $couplages = false, $num = null) {

        $url_params = array();

        $url_params[] = 'user=' . $this->_definition['connection']['user'];
        $url_params[] = 'pwkey=' . $this->_definition['connection']['pwkey'];
        $url_params[] = 'clause=' . $clause;

        if($couplages == true) {
            $url_params[] = 'lescouplages=Y';
        }

        if($num != null) {
            $url_params[] = 'rfrom=1';
            $url_params[] = 'rto=' . $num;
        }

        $url_params = implode('&', $url_params);

        $url_params .= $this->_definition['listing']['url_params'];
        $url_params .= $params;

        // on sépare à nouveau, pour l'encodage (pour prendre en compte les params qui ont été entrés en groupe dans les configs)
        $url_params = explode('&', $url_params);

        array_walk($url_params, function(&$param){
            $tmp = explode('=', $param);
            $param = $tmp[0] . "=" . urlencode($tmp[1]);
        });

        $url_params = implode('&', $url_params);

        $url = $this->_definition['connection']['base_url'] . '/' . $this->_definition['listing']['script'] . '?' . $url_params;

        return $url;
    }

    /**
     * Helper : build the fiche URL
     *
     * @param string $produit
     * @param string $language
     * @param string $nomenclature
     *
     * @return string : absolute URL
     */
    private function _getFicheURL($produit, $language, $nomenclature) {

        $url_params = array();

        $url_params[] = 'user=' . $this->_definition['connection']['user'];
        $url_params[] = 'pwkey=' . $this->_definition['connection']['pwkey'];
        $url_params[] = 'SCHEMA=' . $this->_getLeiSchema($language);
        $url_params[] = 'produit=' . $produit;
        $url_params[] = 'editer=' . $nomenclature;

        $url_params = implode('&', $url_params);

        $url_params .= $this->_definition['fiche']['url_params'];

        $url = $this->_definition['connection']['base_url'] . '/' . $this->_definition['fiche']['script'] . '?' . $url_params;

        return $url;
    }

    /**
     * Helper : build the schema string
     *
     * @param type $language
     * @return type
     */
    private function _getLeiSchema($language) {
        // on utilisera le schema 'WEBACCESS' également pour les langues qui n'ont pas été paramétrées (les champs de settings ne pouvant être #required)
        return 'WEBACCESS' . (!empty($this->_definition['schema'][$language]) ? '_' . strtoupper($this->_definition['schema'][$language]) : '');
    }

    /**
    * Retrieve fields from definition URL
    * @param array $data : the XML in array format
    * @return array
    */
    private function _getFieldsDefinitionFromData($data) {
        $fields_list = array();

        // certains champs ne sont pas présents dans la nomenclature (faute du LEI) :
        $criteres = array_filter((array)$data['Nomenclature'][0]['Criteres']) + (array)$data['Criteres'];
        unset($data['Nomenclature']);

        $prestataire = array_filter((array)$data['Prestataire'][0]);
        unset($data['Prestataire']);

        // si les horaires existent, on n'en a pas besoin pour le moment
        unset($data['Horaires']);

        //----- les champs "root" : ils n'ont pas de label et ne sont pas typés.
        // ils ont une valeur, que l'on utilisera que pour l'affichage
        foreach($data as $field => $value) {

            if(is_array($value)) {continue;}

            $fields_list[$field] = array(
                'label' => $field,
                'type' => 'textfield',
                'values' => !empty($value) ? array($value) : array(),
                'prevent_sorting' => true,
            );
        }

        //----- les infos prestataires
        if(!empty($prestataire)) {
            $fields_list['presta'] = array(
                'label' => "Prestataire",
                'type' => 'group',
                'prevent_sorting' => true,
            );

            foreach($prestataire as $field => $value) {

                $fields_list[$field . '__presta'] = array(
                    'label' => $field,
                    'type' => 'textfield', // à l'instar des champs root : pas de label, et pas de type, et juste une value informative
                    'group' => 'presta',
                    'values' => !empty($value) ? array($value) : array(),
                );
            }
        }

        //----- les critères

        $lei_types_mapping = array(
            '1' => 'integer',
            '2' => 'textfield',
            '3' => 'date',
            '4' => 'image', // chemin (url)
            '5' => 'decimal', // à l'origine, c'est un type monétaire
            '6' => 'file', // url
        );

        foreach($criteres as $index => $critere) {

            $type = !empty($lei_types_mapping[$critere['CRITERE_TYPEVAL']]) ? $lei_types_mapping[$critere['CRITERE_TYPEVAL']] : 'textfield';

            switch($critere['CRITERE_QUALITATIF']) {

                case 0: // valeur unique : qualitatif (ex : descriptif)

                    $fields_list[(string)$critere['CRITERE']] = array(
                        'label' => $critere['CRITERE_NOM'],
                        'type' => $type,
                    );

                    if(array_key_exists($critere['CRITERE'], $data['Criteres'])) {
                        $fields_list[(string)$critere['CRITERE']]['values'] = array($data['Criteres'][$critere['CRITERE']]['Modalites'][0]['VALEUR']);
                    }

                    break;

                case 1: // checkbox (on/off) ou select/radio : modalité unique (ex : animaux acceptés, ou classement hôtels)

                    if(count($critere['Modalites']) == 1) { // checkbox (on/off)

                        $modalite = array_keys($critere['Modalites']);

                        $fields_list[(string)$critere['CRITERE']] = array(
                            'label' => $critere['CRITERE_NOM'],
                            'type' => 'onoff',
                            'values' => array(
                                $critere['CRITERE_NOM'], // seulement affiché à titre informatif
                            ),
                        );

                    } else { // select/radio

                        $fields_list[(string)$critere['CRITERE']] = array(
                            'label' => $critere['CRITERE_NOM'],
                            'type' => 'select',
                        );

                        foreach($critere['Modalites'] as $modalite) {
                            $fields_list[(string)$critere['CRITERE']]['values'][$modalite['MODALITE']] = $modalite['MODALITE_NOM'];
                        }
                    }

                    break;

                case -1: // checkboxes : modalité multiple (ex : équipements)

                    $fields_list[(string)$critere['CRITERE']] = array(
                        'label' => $critere['CRITERE_NOM'],
                        'type' => 'checkboxes',
                    );

                    foreach($critere['Modalites'] as $modalite) {
                        $fields_list[(string)$critere['CRITERE']]['values'][$modalite['MODALITE']] = $modalite['MODALITE_NOM'];
                    }

                    break;

                case 2: // groupe de plusieurs champs : modalité valuée : plusieurs entrées utilisateur (ex : groupe de tarifs, de photos)

                    $fields_list[(string)$critere['CRITERE']] = array(
                        'label' => $critere['CRITERE_NOM'],
                        'type' => 'group',
                    );

                    foreach($critere['Modalites'] as $modalite) {

                        $fields_list[(string)$modalite['MODALITE'] . '__' . $critere['CRITERE']] = array(
                            'label' => $modalite['MODALITE_NOM'],
                            'type' => $type,
                            'group' => (string)$critere['CRITERE'],
                        );

                        if(array_key_exists($critere['CRITERE'], $data['Criteres'])
                                && array_key_exists($modalite['MODALITE'], $data['Criteres'][$critere['CRITERE']]['Modalites'])) {
                            $fields_list[(string)$modalite['MODALITE'] . '__' . $critere['CRITERE']]['values'] = array(
                                $data['Criteres'][$critere['CRITERE']]['Modalites'][$modalite['MODALITE']]['VALEUR'],
                            );
                        }
                    }

                    break;
            }
        }

        return $fields_list;
    }

    private function _getFieldsValuesFromData($data, &$fields, &$title, $language, $node) {

        $hook_modules = module_implements('lei_importer_values');

        $criteres = $data['Criteres'];
        unset($data['Criteres']);

        $prestataire = $data['Prestataire'][0];
        unset($data['Prestataire']);

        $schedules = !empty($data['Horaires']) ? $data['Horaires'] : array();
        unset($data['Horaires']);

        $title_string = "";

        // les champs "root"
        foreach($data as $field => $value) {
            if(empty($title_string) && in_array($field, $title)) {
                $title_string = $value;
            }

            if(array_key_exists($field, $fields)) {

                // si l'admin a choisi le type 'date', une conversion est nécessaire, à partir d'un des deux formats LEI possibles.
                if($fields[$field]['type'] == 'date') {
                    $date = DateTime::createFromFormat('d/m/Y H:i:s', $value);

                    if($date == false) { // on tente la création sans les heures
                        $date = DateTime::createFromFormat('d/m/Y', $value);
                    }

                    if($date != false) {
                        $value = $date->format('Y-m-d 00:00:00');
                    }
                }

                $fields[$field]['values'][] = $value;
            }
        }

        // les champs "prestataire"
        foreach((array)$prestataire as $field => $value) {

            $field .= '__presta';

            if(empty($title_string) && in_array($field, $title)) {
                $title_string = $value;
            }

            if(array_key_exists($field, $fields)) {

                // si l'admin a choisi le type 'date', une conversion est nécessaire, à partir d'un des deux formats LEI possibles.
                if($fields[$field]['type'] == 'date') {
                    $date = DateTime::createFromFormat('d/m/Y H:i:s', $value);

                    if($date == false) { // on tente la création sans les heures
                        $date = DateTime::createFromFormat('d/m/Y', $value);
                    }

                    if($date != false) {
                        $value = $date->format('Y-m-d 00:00:00');
                    }
                }

                $fields[$field]['values'][] = $value;
            }
        }

        // les horaires
        foreach((array)$schedules as $schedule) {

            foreach((array)$schedule['Horaire'] as $horaire) {
                if(!empty($horaire['PERIODE_DU'])) {
                    $date = DateTime::createFromFormat('d/m/Y', $horaire['PERIODE_DU']);
                    $value = $date->format('Y-m-d 00:00:00');
                }

                if(!empty($horaire['PERIODE_AU'])) {
                    $date = DateTime::createFromFormat('d/m/Y', $horaire['PERIODE_AU']);
                    $value = (array)$value;
                    $value[] = $date->format('Y-m-d 00:00:00');
                }

                $fields['schedule_dates']['values'][] = $value;
            }
        }
        foreach($hook_modules as $module) {
            $function = $module . '_lei_importer_values';
            $function('schedules', $schedule['Horaire'], $fields, $node); // passage par référence
        }

        foreach($criteres as $critere) {

            if($critere['CRITERE_QUALITATIF'] == 2) {
                // groupe de plusieurs champs : modalité valuée : plusieurs entrées utilisateur (ex : groupe de tarifs, de photos)
                // le nom du champ Drupal est constitué par le nom de la modalité (et est parfois suffixé avec le nom du groupe)
                foreach($critere['Modalites'] as $modalite) {

                    // Les champs de type "date" (type 3) du LEI, sont au format : 'd/m/Y H:i:s'
                    if($critere['CRITERE_TYPEVAL'] == 3) {

                        // on les convertit au format DateField
                        $date = DateTime::createFromFormat('d/m/Y H:i:s', $modalite['VALEUR']);

                        if($date == false) {// on tente la création sans les heures
                            $date = DateTime::createFromFormat('d/m/Y', $modalite['VALEUR']);
                        }

                        if($date != false) {
                            $valeur = $date->format('Y-m-d H:i:s');
                        }

                    } else {
                        $valeur = $modalite['VALEUR'];
                    }

                    if(empty($title_string) && in_array($modalite['MODALITE'], $title)) {
                        $title_string = $valeur;
                    }

                    if(array_key_exists($modalite['MODALITE'], $fields)) {

                        $field_name = $modalite['MODALITE'];

                    } elseif(array_key_exists($modalite['MODALITE'] . '__' . $critere['CRITERE'], $fields)) {

                        $field_name = $modalite['MODALITE'] . '__' . $critere['CRITERE'];

                    } else {

                        continue;
                    }

                    $fields[$field_name]['values'][] = $valeur;

                    if($language != language_default('language') && module_exists('i18n_field')) {
                        $field_definition = field_info_field($fields[$field_name]['field']);
                        $field_instance = field_info_instance('node', $fields[$field_name]['field'], $node->type);

                        $label_translated = i18n_field_translate_property($field_instance, 'label', $language);

                        if($modalite['MODALITE_NOM'] != $label_translated && $modalite['MODALITE_NOM'] != $field_instance['label']) {
                            i18n_string_translation_update(
                                'field:' . $fields[$field_name]['field'] . ':' . $node->type . ':label',
                                $modalite['MODALITE_NOM'],
                                $language,
                                $field_instance['label']
                            );
                        }
                    }
                }

            } else {

                if(array_key_exists($critere['CRITERE'], $fields)) {

                    $allowed_values = array();

                    foreach($critere['Modalites'] as $modalite) {

                        if(empty($title_string) && in_array($modalite['MODALITE'], $title)) {
                            $title_string = !empty($modalite['VALEUR']) ? $modalite['VALEUR'] : $modalite['MODALITE'];
                        }

                        // champ avec valeur autorisées, à l'exception des checkboxes on/off dont les valeurs autorisées
                        // ne doivent plus être alétérées après la définition initiale
                        if((!array_key_exists('VALEUR', $modalite) || empty($modalite['VALEUR'])) && $fields[$critere['CRITERE']]['type'] != 'onoff') {
                            $allowed_values[$modalite['MODALITE']] = $modalite['MODALITE_NOM'];
                        }

                        // pour les modalites qui ne sont pas des entrées utilisateur, il n'y a pas de valeur :
                        // c'est le fait que la modalité soit présente qui constitue une valeur.
                        $fields[$critere['CRITERE']]['values'][] = !empty($modalite['VALEUR']) ? $modalite['VALEUR'] : $modalite['MODALITE'];
                    }

                    $field_definition = field_info_field($fields[$critere['CRITERE']]['field']);

                    if($language != language_default('language') && module_exists('i18n_field')) {

                        if(module_exists('i18n_field')) {
                            $field_instance = field_info_instance('node', $fields[$critere['CRITERE']]['field'], $node->type);

                            $label_translated = i18n_field_translate_property($field_instance, 'label', $language);

                            if($critere['CRITERE_NOM'] != $label_translated && $critere['CRITERE_NOM'] != $field_instance['label']) {

                                i18n_string_translation_update(
                                    'field:' . $fields[$critere['CRITERE']]['field'] . ':' . $node->type . ':label',
                                    $critere['CRITERE_NOM'],
                                    $language,
                                    $field_instance['label']
                                );

                                if($fields[$critere['CRITERE']]['type'] == 'onoff') {

                                    i18n_string_translation_update(
                                        'field:' . $fields[$critere['CRITERE']]['field'] . ':#allowed_values:1',
                                        $critere['CRITERE_NOM'],
                                        $language,
                                        $field_definition['settings']['allowed_values'][1]
                                    );
                                }
                            }
                        }
                    }

                    if(!empty($allowed_values)) {

                        if(array_key_exists('allowed_values', $field_definition['settings'])) {

                            // 06/08/2015 : pourquoi ai-je fait ça ? Au vu de la condition ci-dessus, $field_definition est FORCEMENT non-vide quand
                            // on atteind cette ligne de code :
                            $field_definition = empty($field_definition) ? field_info_field($fields[$critere['CRITERE']]['field']) : $field_definition;

                            if($language == language_default('language')) {
                                                                
                                $new_allowed_values = array_diff_key($allowed_values, $field_definition['settings']['allowed_values']);
                                
                                // on ne traite que s'il y a des valeurs inédites
                                if(!empty($new_allowed_values)) {
                                
                                    $field_definition['settings']['allowed_values'] += $allowed_values;
                                    asort($field_definition['settings']['allowed_values']);
                                    field_update_field($field_definition);
                                }

                            } elseif(module_exists('i18n_field')) {

                                // on ne conserve que ce qui est réellement une traduction
                                $translations = array_diff_assoc($allowed_values, $field_definition['settings']['allowed_values']);

                                // on écarte les valeurs autorisées inédites. Celles-ci doivent être traitées en langage par défaut.
                                $translations = array_intersect_key($allowed_values, $field_definition['settings']['allowed_values']);

                                if(!empty($translations)) {

                                    $existing = i18n_field_translate_allowed_values($field_definition, $language);

                                    foreach($translations as $allowed_value => $label) {

                                        // si la chaîne a été traduite manuellement pour cause d'absence de traduction LEI, cette traduction ne doit pas être écrasée.
                                        if($existing[$allowed_value] == $label || $label == $field_definition['settings']['allowed_values'][$allowed_value]) {continue;}

                                        i18n_string_translation_update(
                                            'field:' . $fields[$critere['CRITERE']]['field'] . ':#allowed_values:' . $allowed_value,
                                            $label,
                                            $language,
                                            $field_definition['settings']['allowed_values'][$allowed_value]
                                        );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        $title = !empty($title_string) ? $title_string : t("No title") . " (fiche : " . $data['PRODUIT'] . ")"; // LOG :
    }

    /**
     * Helper : convert a simpleXmlElement in an array well formed.
     * This override makes criteres and modalites to be indexed by their name. So it renders possible to join informations
     * of the two sub-arrays 'Criteres' and 'Nomenclature'
     *
     * @param simpleXmlElement $xml
     *
     * @return array
     */
    protected function _xml2array(simpleXmlElement $xml)
    {
        $arr = array();

        foreach ($xml->children() as $r) {

            $attributes = array();

            foreach($r->attributes() as $attr => $val) {
                $attributes[(string)$attr] = (string)$val;
            }

            if(count($r->children()) == 0) {

                if(array_key_exists('CLEF', $attributes)) {
                    $arr[$r->getName()][$attributes['CLEF']] = strval($r); // nomenclature du masque de saisie
                } else {
                    $arr[$r->getName()] = strval($r);
                }

            } else {
                $return = $this->_xml2array($r);

                if(!empty($attributes)) {
                    $return['attributes'] = $attributes;
                }

                if(array_key_exists('CRITERE_QUALITATIF', $return)) {
                    $arr[$r->getName()][$return['CRITERE']] = $return;
                } elseif(array_key_exists('MODALITE', $return)) {
                    $arr[$r->getName()][$return['MODALITE']] = $return;
                } else {
                    $arr[$r->getName()][] = $return;
                }

            }
        }

        return $arr;
    }
}