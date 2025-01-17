<?php

class SoFeedsAgentConnectorPericles extends SoFeedsAgentConnectorAbstract
{
    const PARENT_NODE = 'parent_node';
    const CHILD_NODE = 'child_node';

    private $_unzip_absolute_path = '';

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

    function _get_types_offres($index = null) {
        $types = array(
            1 => "vente appart",
            2 => "vente maison",
            3 => "vente terrain",
            4 => "vente immeuble",
            5 => "vente local",
            6 => "vente fdc",
            7 => "vente parking",
            11 => "location appart",
            12 => "location mais",
            13 => "location local",
            14 => "location parking",
            21 => "programme",
        );

        return $index === null ? $types : $types[$index];
    }

    public function connectorConfigurationForm(array &$form, array &$form_state, $touch_form = false) {

        if($touch_form == true) {
            return true;
        }

        $form = array(
            '#type' => 'fieldset',
            '#title' => "Périclès",

            'upload_directory' => array(
                '#type' => 'textfield',
                '#title' => t("Archives upload directory"),
                '#field_prefix' => "public://",
                '#default_value' => array_key_exists('upload_directory', $this->_definition) ? $this->_definition['upload_directory'] : "",
                '#required' => true,
            ),

            'image_mode' => array(
                '#type' => 'radios',
                '#title' => t("Images update mode"),
                '#options' => array(
                    'update' => t("Update mode : images with new name in archive will be added to node's existing ones, and those with existing name will replace old ones."),
                    'replace' => t("Replace mode : images in archive will replace whole node's existing images."),
                ),
                '#default_value' => array_key_exists('image_mode', $this->_definition) ? $this->_definition['image_mode'] : 'replace',
            ),
        );

        return $form;
    }

    public function importerFeedConfigurationForm(array &$form, array $configuration) {

        $form['#title'] = "Périclès";

        $form['filename'] = array(
            '#type' => 'textfield',
            '#title' => t("Archive name"),
            '#description' => t("File name without extension"),
            '#field_prefix' => "public://" . $this->_definition['upload_directory'] . "/",
            '#field_suffix' => ".ZIP",
            '#default_value' => $configuration['filename'],
        );

        $form['type_offre'] = array(
            '#type' => 'select',
            '#title' => t("Offers type"),
            '#options' => $this->_get_types_offres(),
            '#default_value' => $configuration['type_offre'],
        );
    }

    public function importerDisplayFeedInfos(array $configuration) {

        if(empty($configuration)) {return;}

        $output = "";

        $output .= "<strong>" . t("Offers type") . " : </strong>" . ucfirst($this->_get_types_offres($configuration['type_offre'])) . "<br />";

        $file = 'public://' . $this->_definition['upload_directory'] . '/' . $configuration['filename'] . '.ZIP';

        if(is_file($file)) {

            $output .= "<strong>" . t("File") . " : </strong>" . l($file, file_create_url($file), array('attributes' => array('target' => '_blank'))) . "<br />";

            $last_update = filemtime($file);
            $last_update_time = DateTime::createFromFormat('U', $last_update)->setTimezone(new DateTimeZone('Europe/Paris'))->format('d/m/Y - H:i:s');
            $output .= "<strong>" . t("State") . " : </strong>" . t("Updated on @datetime", array('@datetime' => $last_update_time)) . "<br />";

        } else {

            $output .= "<strong>" . t("File") . " : </strong>" . $file . "<br />";
            $output != "<strong>" . t("State") . " : </strong>" . t("File doesn't exist yet") . "<br />";
        }

        return $output;
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

        $directory = 'public://' . $this->_definition['upload_directory'];

        $dir_handle = opendir($directory);

        $files_options = array();

        while($filename = readdir($dir_handle)) {
            if(is_file($directory . '/' . $filename)) {
                $file_name_parts = explode('.', $filename);
                $files_options[$file_name_parts[0]] = $filename;// on retire l'extension
            }
        }

        ksort($files_options);

        $step_elements['file'] = array(
            '#type' => 'select',
            '#title' => t("Archive to use"),
            '#options' => $files_options,
        );

        $step_elements['item_index'] = array(
            '#type' => 'textfield',
            '#title' => t("Number of items to skip"),
            '#default_value' => 0,
            '#size' => 2,
        );
    }

    public function generatorListingParamsStepSubmission(array &$form_state, $obsolescence) {

        _so_feedsagent_register_dependency($form_state, 'file');
        _so_feedsagent_register_dependency($form_state, 'index');
        _so_feedsagent_register_dependency($form_state, 'feed_fields', array('connector', 'file', 'index'));

        // si les paramètres propre au connecteur n'ont pas encore été définis, ou que ces paramètres ont changé
        if(empty($form_state['wizzard_params']['params']['connector']) || $obsolescence == true) {

            $form_state['wizzard_params']['params']['connector'] = $form_state['values']['step_elements'];
            _so_feedsagent_invalidate_dependencies($form_state, array('file', 'index'));
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
                                "A Périclès virtual type named '@name' and matching the primary type '@primary' already exists.<br />It's mapped on '@type' Drupal's content type.",
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
                        _so_feedsagent_register_dependency($form_state, 'id', array('connector', 'file', 'index'));
                        _so_feedsagent_invalidate_dependencies($form_state, array('id'));
                        $form_state['wizzard_params']['id'] = $content_types_definition->id;

                            drupal_set_message(
                            t(
                                "A Périclès virtual type named '@name' and matching the primary type '@primary' already exists.<br />Since it's not mapped on any Drupal's content type, it can be redefined.",
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

        $filename = $wizzard_params['params']['connector']['file'];
        $item_index = $wizzard_params['params']['connector']['item_index'];

        $all_data = $this->_get_xml_data($filename, false, true);

        $all_data_flat = array();

        foreach($all_data['BIEN'] as $no_asp => $children) {
            foreach($children as $index => $child) {
                $all_data_flat[] = $child;
            }
        }

        $data = array_slice($all_data_flat, $item_index, 1);
        $data = $data[0];

        $fields_list = array(
            $this->_definition['id'] => array(
                'label' => $this->_definition['label'],
                'type' => 'group',
                'locked' => true,
            ),
            'NO_ASP' => array(
                'label' => t("ASP num."),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['NO_ASP']),
                'locked' => true,
            ),
            'TYPE_OFFRE' => array(
                'label' => t("Offer type"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['TYPE_OFFRE']),
                'locked' => true,
            ),
            'DATE_OFFRE' => array(
                'label' => t("Offer date"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['DATE_OFFRE']),
                'locked' => true,
            ),
            'DATE_MODIF' => array(
                'label' => t("Date edit."),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['DATE_MODIF']),
                'locked' => true,
            ),
            'NODE_HIERARCHY' => array(
                'label' => t("Node hierarchy"),
                'type' => 'select',
                'group' => $this->_definition['id'],
                'values' => array(
                    self::PARENT_NODE => t("Parent node"),
                    self::CHILD_NODE => t("Child node"),
                ),
                'multiple' => false,
                'locked' => true,
            ),

            'GEOLOCATION' => array(
                'label' => t("Geolocation"),
                'type' => 'group',
                'prevent_sorting' => true,
            ),

            'LONGITUDE' => array(
                'label' => t("Longitude"),
                'type' => 'textfield',
                'group' => 'GEOLOCATION',
            ),

            'LATITUDE' => array(
                'label' => t("Latitude"),
                'type' => 'textfield',
                'group' => 'GEOLOCATION',
            ),

            'IMAGES' => array(
                'label' => t("Images"),
                'type' => 'image',
                'multiple' => true,
                'prevent_sorting' => true,
            ),
        );

        $wizzard_params['primary_type'] = $data['TYPE_OFFRE'] . '@' . $this->_definition['id'];
        $wizzard_params['virtual_name'] = ucfirst($this->_get_types_offres($data['TYPE_OFFRE']));

        unset($data['NO_ASP']);
        unset($data['TYPE_OFFRE']);
        unset($data['DATE_OFFRE']);
        unset($data['DATE_MODIF']);

        foreach($data as $field => $value) {
            $fields_list[$field] = array(
                'label' => $field,
                'type' => 'undefined',
                'values' => !empty($value) ? array(substr(md5($value), 0, 8) => $value) : array(),
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

    public function importerGetFeedListing(array $configuration) {

        $filename = $configuration['filename'];
        $all_data = $this->_get_xml_data($filename, true, true);
        $data = $all_data['BIEN'];

        $listing = array();
        $weight = 0;

        foreach($data as $no_asp => $children) {

            $children_data = reset($children);

            if($children_data['TYPE_OFFRE'] != $configuration['type_offre']) {continue;}

            $datemaj = DateTime::createFromFormat('d/m/Y', $children_data['DATE_MODIF']);

            // fiche mère
            $listing[$no_asp] = array(
                'id' => $no_asp,
                'parent_id' => null,
                'primary_type' => $children_data['TYPE_OFFRE'] . '@' . $this->_definition['id'],
                'primary_type_name' => ucfirst($this->_get_types_offres($children_data['TYPE_OFFRE'])),
                'date' => $datemaj->format('U'),
                'weight' => $weight++,
            );

            foreach($children as $no_dossier => $child) {

                $listing[$no_asp . '_' . $no_dossier] = array(
                    'id' => $no_asp . '_' . $no_dossier,
                    'parent_id' => $no_asp,
                    'primary_type' => $children_data['TYPE_OFFRE'] . '@' . $this->_definition['id'],
                    'primary_type_name' => ucfirst($this->_get_types_offres($children_data['TYPE_OFFRE'])),
                    'date' => $datemaj->format('U'),
                    'weight' => $weight++,
                );
            }
        }

        return $listing;
    }

    public function importerGetFeedValues($node, $item_id, array &$title, $language, array $configuration, array &$fields, stdClass $feed_definition) {

        $filename = $feed_definition->params['connector']['filename'];
        $all_data = $this->_get_xml_data($filename, true);

        $parsed_item_id = explode('_', $item_id);
        $no_asp = $parsed_item_id[0];

        $title_suffix = "";

        // child node
        if(count($parsed_item_id) == 2) {
            $no_dossier = $parsed_item_id[1];
            $data = $all_data['BIEN'][$no_asp][$no_dossier];
            $title_suffix = " (" . $no_dossier . ")";
            $node_hierarchy = self::CHILD_NODE;
            $fields['NODE_HIERARCHY']['values'] = array($node_hierarchy);

        // parent node
        } else {
            $data = reset($all_data['BIEN'][$no_asp]);
            $node_hierarchy = self::PARENT_NODE;
            $fields['NODE_HIERARCHY']['values'] = array($node_hierarchy);
        }

        unset($data['NODE_HIERARCHY']);

        $fields_types = so_feedsagent_get_available_fields_types();

        $allowed_values = array();

        // initialisation du tableau des valeurs autorisées (indexé par Drupal's fields)
        foreach($fields as $field => $infos) {
            if($fields_types[$infos['type']]['allowed_values'] == true) {
                $allowed_values[$infos['field']] = array();
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

                    if(!empty($value)) {
                        $hashed_value = substr(md5($value), 0, 8);
                        $allowed_values[$fields[$field]['field']][$hashed_value] = $value;
                        $fields[$field]['values'][] = $hashed_value;
                    }

                } else {

                    if($fields[$field]['type'] == 'date') {

                        //Le format Périclès semble être : d/m/Y
                        $date = DateTime::createFromFormat('d/m/Y', $value);
                        $value = $date->format('Y-m-d 00:00:00');

                    }

                    $fields[$field]['values'] = array($value);
                }
            }
        }

        foreach($allowed_values as $field => $infos) {

            //ces valeurs-là sont et doivent rester des constantes.
            if($field == $fields['NODE_HIERARCHY']['field']) {
                continue;
            }

            $field_infos = field_info_field($field);
            $field_infos['settings']['allowed_values'] += $infos;
            asort($field_infos['settings']['allowed_values']);
            field_update_field($field_infos);
        }

        //--- IMAGES
        if($node_hierarchy == self::PARENT_NODE && array_key_exists('IMAGES', $fields)) {

            $images_file_pattern = $this->_unzip_absolute_path . '/' . $data['CODE_SOCIETE'] . '-' . $data['CODE_SITE'] . '-' . $data['NO_ASP'] . '-*.jpg';
            $images_paths = array();
            $drupal_field = $fields['IMAGES']['field'];

            $target_filename_prefix = preg_replace('#^field#', $node->nid, $drupal_field) . '_';

            foreach(glob($images_file_pattern) as $image_path) {
                $source_path = str_replace(drupal_realpath('public://') . '/', 'public://', $image_path);

                $target_filename = $target_filename_prefix . pathinfo($source_path, PATHINFO_FILENAME) . '.' . pathinfo($source_path, PATHINFO_EXTENSION);
                $images_paths[$target_filename] = $source_path;
            }

            if(!empty($images_paths)) {

                $original_node = node_load($node->nid, null, true);//sans le reset, les champs syndiqués sont vides. POURQUOI ?

                //si le champ d'origine est vide, ou le mode des images est défini sur 'replace'
                if(empty($original_node->{$drupal_field})
                    || $this->_definition['image_mode'] == 'replace') {

                    // cas du 'replace'
                    if(!empty($original_node->{$drupal_field})) {
                        //on supprime le cache des images anciennes images car les nouvelles auront très probablement le même nom
                        foreach($original_node->{$drupal_field}[$node->language] as $field_value) {
                            image_path_flush($field_value['uri']);
                        }
                    }

                //le champ d'origine n'est pas vide, ou le mode des images est défini sur 'update'
                } else {
                    //on transfert les anciennes valeurs du node d'origine vers le node en cours d'écriture
                    $node->{$drupal_field} = $original_node->{$drupal_field};

                    //et on supprime les entrées des images existantes (sinon : doublons)
                    foreach($node->{$drupal_field}[$node->language] as $index => $field_value) {
                        if(array_key_exists($field_value['filename'], $images_paths)) {
                            //on supprime le cache de l'ancienne image
                            image_path_flush($field_value['uri']);
                            unset($node->{$drupal_field}[$node->language][$index]);
                        }
                    }
                }

                foreach($images_paths as $image_path) {
                    $fields['IMAGES']['values'][] = $image_path;
                }

            } else {
                //les images existantes ne doivent pas être écrasées
                unset($node->{$drupal_field});
            }
        }

        //--- Geolocation
        if($node_hierarchy == self::PARENT_NODE
            && array_key_exists('LONGITUDE', $fields)
            && array_key_exists('LATITUDE', $fields)) {

            $coordinates = $this->_geocode(
                $data['VILLE_OFFRE'],
                'fr',
                $data['CP_OFFRE'],
                $data['ADRESSE1_OFFRE'],
                $data['ADRESSE2_OFFRE']
            );

            if($coordinates != false) {
                $fields['LONGITUDE']['values'] = array($coordinates['lng']);
                $fields['LATITUDE']['values'] = array($coordinates['lat']);
            }
        }

        $title = $title_string . $title_suffix;

        return;
    }

    /**
    * Get address coordinates from Google
    *
    * @param string $city
    * @param string $country
    * @param string $cp
    * @param string $address1
    * @param string $address2
    * @return mixed : array of lat/lng or false
    */
    function _geocode($city = "", $country = "", $cp = "", $address1 = "", $address2 = "") {

        $place = array();

        if(!empty($address1) || !empty($address2)) {
            $place[] = $address1 . (!empty($address2) ? " " . $address2 : "");
        }

        if(!empty($cp)) {
            $place[] = $cp;
        }

        if(!empty($city)) {
            $place[] = $city;
        }

        if(!empty($country)) {
            $place[] = $country;
        }

        $place = urlencode(implode(',', $place));

        $url = "http://maps.google.com/maps/api/geocode/json?sensor=false&address=" . $place;
        $response = file_get_contents($url);

        if(!empty($response)) {

            $response = json_decode($response, true);

            if(!array_key_exists('status', $response) || $response['status'] != "OK") {
                return false;
            }

            if(empty($response['results'])) {
                return false;
            }

            return $response['results'][0]['geometry']['location'];
        }

        return false;
    }

    function _get_xml_data($filename, $filter = true, $reset_cache = false) {

        $connector_id = $this->_definition['id'];
        $upload_directory = drupal_realpath('public://' . $this->_definition['upload_directory']);
        $unzip_directory = $this->_inflate_archive($filename, $upload_directory, $connector_id);
        $this->_unzip_absolute_path = $upload_directory . '/' . $unzip_directory;

        $cid = 'sfa_pericles:' . $unzip_directory . ':' . ($filter == true ? 'filtered' : 'unfiltered');

        $cached = cache_get($cid);

        if(empty($cached) || $reset_cache == true) {

             $xml = simplexml_load_file($this->_findXMLFile($upload_directory . '/' . $unzip_directory), null, LIBXML_NOCDATA);
             $data = $this->_xml2array($xml, $filter);
             cache_set($cid, $data);

             return $data;
             
        } else {
             return $cached->data;
        }
    }

    /**
     * Unzip function
     *
     * @param string $filename : without extension
     * @param string $upload_directory : with scheme
     * @param string $connector_id
     *
     * @return string : the absolute path to the files.
     */
    function _inflate_archive($filename, $upload_directory, $connector_id) {

        $file_time = filemtime($upload_directory . '/' . $filename . '.ZIP');
        $file_date = DateTime::createFromFormat('U', $file_time)->setTimezone(new DateTimeZone('Europe/Paris'))->format('Ymd');
        $unzip_directory = $file_date . '_' . $connector_id;
        $target_directory = $upload_directory . '/' . $unzip_directory;

        if(!is_dir($target_directory)) {
            $archive = new ZipArchive();
            $archive->open($upload_directory .'/' . $filename . '.ZIP');
            $archive->extractTo($target_directory);
            $archive->close();
        }

        return $unzip_directory;
    }

    protected function _findXMLFile($directory) {
        $file_array = glob($directory . '/*.XML');

        if(empty($file_array)) {
            $file_array = glob($directory . '/*.xml');
        }

        return !empty($file_array) ? $file_array[0] : false;
    }

   /**
     * le tableau sera indexé par NO_ASP
     *
     * @param simpleXmlElement $xml
     * @param boolean $filter : doit-on filtrer les lots sans n° de dossier ? (défaut : true)
     * @return array
     */
    protected function _xml2array(simpleXmlElement $xml, $filter = true)
    {
        $arr = array();

        foreach ($xml->children() as $r) {

            if(count($r->children()) == 0) {
                $arr[$r->getName()] = strval($r);
            } else {
                $tmp = $this->_xml2array($r, $filter);

                if($filter == true) {
                    if(empty($tmp['NO_DOSSIER'])) {
                        continue;
                    }

                    $arr[$r->getName()][$tmp['NO_ASP']][$tmp['NO_DOSSIER']] = $tmp;
                } else {
                    $arr[$r->getName()][$tmp['NO_ASP']][] = $tmp;
                }
            }
        }

        return $arr;
    }
}

