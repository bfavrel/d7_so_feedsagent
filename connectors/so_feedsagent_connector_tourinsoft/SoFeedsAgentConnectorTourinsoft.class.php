<?php

class SoFeedsAgentConnectorTourinsoft extends SoFeedsAgentConnectorAbstract
{
    public static function getFeatures() {

        return array(
            'implements' => array(
                'generator',
                'importer',
            ),
            'dynamic_allowed_values_only' => true, // les valeurs ne seront pas assignées à la création du champ

            'use_multiple_fields' => true,// TODO : obligatoire - supprimer tout ce qui se rapport à ce paramètre (code, CSS, etc.)
            'use_fields_compatibility' => true, // TODO : obligatoire - supprimer tout ce qui se rapport à ce paramètre (code, CSS, etc.)
        );
    }

    public function connectorConfigurationForm(array &$form, array &$form_state, $touch_form = false) {

        if($touch_form == true) {
            return true;
        }

        if(array_key_exists('data_import', $this->_definition) && !empty($this->_definition['data_import']['script'])) {

            $script_path = DRUPAL_ROOT . '/' . $this->_definition['data_import']['script'];

            if(is_file($script_path)) {

                $file_time = DateTime::createFromFormat('U', filemtime($script_path));
                $file_time->setTimezone(new DateTimeZone("Europe/Paris"));

                $script_description = t("File was found. Last update : @date at @time.", array('@date' => $file_time->format('d/m/Y'), '@time' => $file_time->format('H:i:s')));

            } else {
                $script_description = t("FILE WAS NOT FOUND !");
            }

        } else {
            $script_description = t("After saving, check here to see if file is found.");
        }

        $form = array(
            '#type' => 'vertical_tabs',
            '#title' => "Tourinsoft",
            '#theme_wrappers' => array('vertical_tabs', 'fieldset'),

            'webservice' => array(
                '#type' => 'fieldset',
                '#title' => t("Webservice"),

                'base_url' => array(
                    '#type' => 'textfield',
                    '#title' => t("Base URL"),
                    '#default_value' => array_key_exists('webservice', $this->_definition) ? $this->_definition['webservice']['base_url'] : "http://cdtXX.tourinsoft.com/soft/RechercheDynamique/Syndication/controle/syndication2.asmx",
                    '#size' => 100,
                    '#required' => true,
                ),

                'media_url' => array(
                    '#type' => 'textfield',
                    '#title' => t("Media contents base URL"),
                    '#description' => t("Without trailing slash."),
                    '#default_value' => array_key_exists('webservice', $this->_definition) ? $this->_definition['webservice']['media_url'] : "http://cdtXX.tourinsoft.com/upload",
                    '#required' => true,
                ),


            ),

            'data_import' => array(
                '#type' => 'fieldset',
                '#title' => t("Data import"),

                'script' => array(
                    '#type' => 'textfield',
                    '#title' => t("Import script path"),
                    '#description' => $script_description,
                    '#default_value' => array_key_exists('data_import', $this->_definition) ? $this->_definition['data_import']['script'] : "sites/default/tourinsoft/import_filters.inc",
                ),

                'separator' => array(
                    '#type' => 'textfield',
                    '#title' => t("Values separator"),
                    '#default_value' => array_key_exists('data_import', $this->_definition) ? $this->_definition['data_import']['separator'] : "|",
                    '#size' => 1,
                    '#required' => true,
                ),

                'import_filters' => array(
                    '#type' => 'textarea',
                    '#title' => t("Import filters") . " (" . t("overridable in each feeds") . ")",
                    '#description' => t("One condition per line. Relation between conditions : 'OR'. Format : [field][operator (either '=', '!=', '~' or '!~')][value].<br />Exemple : \"zoneOT~OT Périgueux\", \"categorie!=Excursion en autocar\""),
                    '#default_value' => array_key_exists('data_import', $this->_definition) ? $this->_definition['data_import']['import_filters'] : "",
                ),
            ),
        );
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

    public function generatorListingParamsStep(array &$step_elements, array &$form_state, array $args = array()) {

        if(empty($form_state['values']['step_elements'])) {
            // type virtuel déjà paramétré par le passé
            if(!empty($form_state['wizzard_params']['params']['connector']) && !array_key_exists('feed', $form_state['wizzard_params']['params']['connector'])) {

                $id_module_default = $form_state['wizzard_params']['params']['connector']['id_module'];
                $id_offre_default = $form_state['wizzard_params']['params']['connector']['id_offre'];
                $objettour_code_default = $form_state['wizzard_params']['params']['connector']['objettour_code'];

            // type virtuel créé à la volée par l'importer
            } elseif(!empty($form_state['wizzard_params']['params']['connector']) && array_key_exists('feed', $form_state['wizzard_params']['params']['connector'])) {

                $id_module_default = $form_state['wizzard_params']['params']['connector']['feed']->params['connector']['webservice']['id_module'];
                $id_offre_default = $form_state['wizzard_params']['params']['connector']['item']['id'];

                $objettour = explode('@', $form_state['wizzard_params']['params']['connector']['item']['primary_type']);
                $objettour_code_default = $objettour[0];
            }
        } else {
            $id_module_default = array_key_exists('id_module', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['id_module'] : "";
            $id_offre_default = array_key_exists('id_offre', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['id_offre'] : "";
            $objettour_code_default = array_key_exists('objettour_code', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['objettour_code'] : "";
        }

        $step_elements = array(
            'id_module' => array(
                '#type' => 'textfield',
                '#title' => "idModule",
                '#default_value' => $id_module_default,
                '#size' => 60,
                '#required' => true,
            ),

            'id_offre' => array(
                '#type' => 'textfield',
                '#title' => "idOffre",
                '#default_value' => $id_offre_default,
                '#size' => 20,
                '#required' => true,
            ),

            'objettour_code' => array(
                '#type' => 'textfield',
                '#title' => "OBJETTOUR_CODE",
                '#default_value' => $objettour_code_default,
                '#size' => 10,
                '#required' => true,
            ),
        );

    }

    public function generatorListingParamsStepSubmission(array &$form_state, $obsolescence) {

        _so_feedsagent_register_dependency($form_state, 'id_module');
        _so_feedsagent_register_dependency($form_state, 'id_offre');
        _so_feedsagent_register_dependency($form_state, 'feed_fields', array('connector', 'id_module', 'id_offre'));

        // si les paramètres propre au connecteur n'ont pas encore été définis, ou que ces paramètres ont changé
        if(empty($form_state['wizzard_params']['params']['connector']) || $obsolescence == true) {

            $form_state['wizzard_params']['params']['connector'] = $form_state['values']['step_elements'];
            _so_feedsagent_invalidate_dependencies($form_state, array('id_module', 'id_offre'));
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
                                "A Tourinsoft virtual type named '@name' and matching the primary type '@primary' already exists.<br />It's mapped on '@type' Drupal's content type.",
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
                        _so_feedsagent_register_dependency($form_state, 'id', array('connector', 'id_module', 'id_offre'));
                        _so_feedsagent_invalidate_dependencies($form_state, array('id'));
                        $form_state['wizzard_params']['id'] = $content_types_definition->id;

                            drupal_set_message(
                            t(
                                "A Tourinsoft virtual type named '@name' and matching the primary type '@primary' already exists.<br />Since it's not mapped on any Drupal's content type, it can be redefined.",
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

    public function generatorPopulateFieldsDefinitionsList(array &$wizzard_params) {

        $params = array(
            'idModule' => $wizzard_params['params']['connector']['id_module'],
            'idOffre' => $wizzard_params['params']['connector']['id_offre'],
            'OBJETTOUR_CODE' => $wizzard_params['params']['connector']['objettour_code'],
        );

        $response = $this->_makeSoapRequest($this->_definition['webservice']['base_url'], 'getDetail', 'getDetailResult', $params);
        $xml = simplexml_load_string($response->any, null, LIBXML_NOCDATA);

        // DEV : tester la validité du XML et transmettre les éventuelles infos d'erreur
        $data = $this->_xml2array($xml->Listing->DETAIL);

        // les champs système...
        $fields_list = array(
            // ...que l'on rassemble au sein d'un groupe portant le nom du connecteur
            $this->_definition['id'] => array(
                'label' => $this->_definition['label'],
                'type' => 'group',
                'locked' => true,
            ),

            'ID' => array(
                'label' => "ID",
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['ID']),
                'locked' => true,
            ),
            'DATECREA' => array(
                'label' => t("Created"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['DATECREA']),
                'locked' => true,
            ),
            'DATEMAJ' => array(
                'label' => t("Updated"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['DATEMAJ']),
                'locked' => true,
            ),
        );

        $wizzard_params['primary_type'] = $data['OBJETTOUR_CODE'] . '@' . $this->_definition['id'];
        $wizzard_params['virtual_name'] = $data['OBJETTOUR_LIBELLE'];

        unset($data['OBJETTOUR_CODE']);
        unset($data['ID']);
        unset($data['DATECREA']);
        unset($data['DATEMAJ']);

        foreach($data as $field => $value) {
            $fields_list[$field] = array(
                'label' => $field,
                'type' => 'undefined',
                'values' => !empty($value) ? array($value) : array(),
            );
        }

        $wizzard_params['feed_fields'] = $fields_list;
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

        $form['#title'] = "Tourinsoft";

        $import_filters_default = !empty($configuration) ? $configuration['data_import']['import_filters'] : $this->_definition['data_import']['import_filters'];

        $form = array(
            '#type' => 'vertical_tabs',
            '#title' => "Tourinsoft",
            '#theme_wrappers' => array('vertical_tabs', 'fieldset'),

            'webservice' => array(
                '#type' => 'fieldset',
                '#title' => t("Webservice"),

                'id_module' => array(
                    '#type' => 'textfield',
                    '#title' => "ID Module",
                    '#size' => 60,
                    '#required' => true,
                    '#default_value' => $configuration['webservice']['id_module'],
                ),
            ),

            'data_import' => array(
                '#type' => 'fieldset',
                '#title' => t("Data import"),

                'import_filters' => array(
                    '#type' => 'textarea',
                    '#title' => t("Import filters"),
                    '#description' => t("One condition per line. Relation between conditions : 'OR'. Format : [field][operator (either '=', '!=', '~' or '!~')][value].<br />Exemple : \"zoneOT~OT Périgueux\", \"categorie!=Excursion en autocar\""),
                    '#default_value' => $import_filters_default,
                ),
            ),
        );
    }

    public function importerDisplayFeedInfos(array $configuration) {

        if(empty($configuration)) {return;}

        $output = "";

        $output .= "<strong>ID Module :</strong> " . $configuration['webservice']['id_module'] . "<br />";

        if(!empty($configuration['data_import']['import_filters'])) {
            $output .= "<strong>Filters :</strong> " . str_replace(PHP_EOL, ', ', $configuration['data_import']['import_filters']) . "<br />";
        }

        $output .= "<strong>- " . l(t("See XML"),
                    $this->_definition['webservice']['base_url'] . '/getListing?idModule=' . $configuration['webservice']['id_module'],
                    array(
                        'attributes' => array(
                            'target' => '_blank',
                        ),
                    )) . " -</strong>";

        return $output;
    }

    public function importerGetFeedListing(array $configuration) {

        $params = array(
            'idModule' => $configuration['webservice']['id_module'],
        );

        $response = $this->_makeSoapRequest($this->_definition['webservice']['base_url'], 'getListing', 'getListingResult', $params);
        $xml = simplexml_load_string($response->any, null, LIBXML_NOCDATA);

        if(empty($xml)) {
            return t("Server didn't provide any listing XML data");
        } elseif(empty($xml->Listing)) {
            return t("XML was empty");
        }

        $data = $this->_xml2array($xml->Listing);
        $data = $data['LISTING'];

        // une seule fiche dans le listing : structure différente
        if(array_key_exists('ID', $data)) {
            $data = array($data);
        }

        $import_filters = explode(PHP_EOL, $configuration['data_import']['import_filters']);

        $filters = array();

        foreach($import_filters as $filter) {
            $filter = trim($filter);

            $filter_parts = explode('!=', $filter);

            if(count($filter_parts) == 2) {
                $filters[] = array(
                    'field' => $filter_parts[0],
                    'op' => 'not_equal',
                    'arg' => $filter_parts[1],
                );
                continue;
            }

            $filter_parts = explode('=', $filter);

            if(count($filter_parts) == 2) {
                $filters[] = array(
                    'field' => $filter_parts[0],
                    'op' => 'equal',
                    'arg' => $filter_parts[1],
                );
                continue;
            }

            $filter_parts = explode('!~', $filter);

            if(count($filter_parts) == 2) {
                $filters[] = array(
                    'field' => $filter_parts[0],
                    'op' => 'not_like',
                    'arg' => $filter_parts[1],
                );
                continue;
            }

            $filter_parts = explode('~', $filter);

            if(count($filter_parts) == 2) {
                $filters[] = array(
                    'field' => $filter_parts[0],
                    'op' => 'like',
                    'arg' => $filter_parts[1],
                );
                continue;
            }
        }

        $listing = array();

        foreach($data as $index => $fiche) {

            $filtered = -count($filters);

            foreach($filters as $filter) {

                if(array_key_exists($filter['field'], $fiche)) {

                    switch($filter['op']) {
                        case 'equal':
                            $filtered += ($fiche[$filter['field']] != $filter['arg']);
                            break;

                        case 'not_equal':
                            $filtered += ($fiche[$filter['field']] == $filter['arg']);
                            break;

                        case 'like':
                            $filtered += strpos($fiche[$filter['field']], $filter['arg']) === false;
                            break;

                        case 'not_like':
                            $filtered += strpos($fiche[$filter['field']], $filter['arg']) !== false;
                            break;
                    }
                }
            }

            if(count($filters) > 0 && $filtered == 0) {continue;}

            $datemaj = DateTime::createFromFormat('Y-m-d\TH:i:s.u0P', $fiche['DATEMAJ']);

            if($datemaj == false) {
                $datemaj = DateTime::createFromFormat('Y-m-d\TH:i:sP', $fiche['DATEMAJ']);
            }

            if($datemaj == false) {
                $datemaj = DateTime::createFromFormat('Y-m-d\TH:i:s.uP', $fiche['DATEMAJ']);
            }

            $listing[$fiche['ID']] = array(
                'id' => $fiche['ID'],
                'parent_id' => null,
                'primary_type' => $fiche['OBJETTOUR_CODE'] . '@' . $this->_definition['id'],
                'primary_type_name' => $fiche['OBJETTOUR_LIBELLE'],
                'date' => $datemaj->format('U'),
                'weight' => 0,
            );
        }

        return $listing;
    }

    public function importerGetItemURL($item_id, $language, stdClass $type_definition, stdClass $feed_definition) {

        $config = $type_definition->params['connector'];

        return $this->_definition['webservice']['base_url'] . '/getDetail?' .
                                                            'idModule=' . $feed_definition->params['connector']['webservice']['id_module'] .
                                                            '&idOffre=' . $item_id .
                                                            '&OBJETTOUR_CODE=' . $config['objettour_code'];
    }

    public function importerGetFeedValues($node, $item_id, array &$title, $language, array $configuration, array &$fields, stdClass $feed_definition) {

        $params = array(
            'idModule' => $feed_definition->params['connector']['webservice']['id_module'],
            'idOffre' => $item_id,
            'OBJETTOUR_CODE' => $configuration['objettour_code'],
        );

        $response = $this->_makeSoapRequest($this->_definition['webservice']['base_url'], 'getDetail', 'getDetailResult', $params);

        $xml = simplexml_load_string($response->any, null, LIBXML_NOCDATA);

        if(empty($xml)) {
            return t("Server didn't provide any fiche XML data");
        } elseif(empty($xml->Listing->DETAIL)) {
            return t("XML was empty");
        }

        $data = $this->_xml2array($xml->Listing->DETAIL);

        $fields_types = so_feedsagent_get_available_fields_types();

        $allowed_values = array();

        // initialisation du tableau des valeurs autorisées (indexé par Drupal's fields)
        foreach($fields as $field => $infos) {
            if($fields_types[$infos['type']]['allowed_values'] == true) {
                $allowed_values[$infos['field']] = array();
            }
        }

        $script_path = DRUPAL_ROOT . '/' . $this->_definition['data_import']['script'];
        require_once $script_path;

        foreach($data as $field => $value) {

            if(array_key_exists($field, $fields)) {

                $fields[$field]['filtered_values'] = array();

                $field_filter = 'sfa_' . $field . '_field_filter';
                $type_filter = 'sfa_' . $fields[$field]['type'] . '_type_filter';

                if(is_callable($field_filter)) {

                    $field_filter(
                        $fields[$field]['filtered_values'],
                        $data,
                        $fields,
                        $node,
                        $this->_definition,
                        $configuration
                    );
                } elseif(is_callable($type_filter)) {

                    $type_filter(
                        $fields[$field]['filtered_values'],
                        $field,
                        $data,
                        $fields,
                        $node,
                        $this->_definition,
                        $configuration
                    );
                } else {
                    $fields[$field]['filtered_values'] = explode($this->_definition['data_import']['separator'], $value);
                }
            }
        }

        $title_string = "";

        foreach($data as $field => $value) {
            if(empty($title_string) && in_array($field, $title)) {
                $title_string = $value;
            }

            if(array_key_exists($field, $fields)) {

                $fields[$field]['values'] = array();

                if(array_key_exists($fields[$field]['field'], $allowed_values)) {

                    foreach($fields[$field]['filtered_values'] as $allowed_value) {

                        if(empty($allowed_value)) {continue;}

                        $hashed_value = substr(md5($allowed_value), 0, 8);
                        $allowed_values[$fields[$field]['field']][$hashed_value] = $allowed_value;
                        $fields[$field]['values'][] = $hashed_value;
                    }

                } else {
                    $fields[$field]['values'] = $fields[$field]['filtered_values'];
                }
            }
        }

        foreach($allowed_values as $field => $infos) {
            $field_infos = field_info_field($field);
            $field_infos['settings']['allowed_values'] += $infos;
            asort($field_infos['settings']['allowed_values']);
            field_update_field($field_infos);
        }

        $title = !empty($title_string) ? $title_string : t("No title") . " (fiche : " . $data['ID'] . ")"; // LOG :

        return;
    }

    private function _makeSoapRequest($location, $request, $response, $params = array()) {

        $client = new SoapClient(
            $location . "?WSDL",
            array(
                'soap_version' => SOAP_1_1,
                'trace' => false,
                'exceptions' => false,
            )
        );

        $call = $client->$request($params);

        return $call->$response;
    }
}