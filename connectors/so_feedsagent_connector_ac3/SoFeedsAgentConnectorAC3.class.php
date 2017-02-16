<?php

class SoFeedsAgentConnectorAC3 extends SoFeedsAgentConnectorAbstract
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
            '#type' => 'fieldset',
            '#title' => "Immo-Facile XML aC3",

            'upload_directory' => array(
                '#type' => 'textfield',
                '#title' => t("Archives upload directory"),
                '#description' => t("Without trailing slash"),
                '#field_prefix' => "public://",
                '#default_value' => array_key_exists('upload_directory', $this->_definition) ? $this->_definition['upload_directory'] : "",
                '#required' => true,
            ),
        );

        return $form;
    }

    public function importerFeedConfigurationForm(array &$form, array $configuration) {

        $form['#title'] = "Immo-Facile XML aC3";

        $form['filename'] = array(
            '#type' => 'textfield',
            '#title' => t("Archive name"),
            '#description' => t("File name without extension"),
            '#field_prefix' => "public://" . $this->_definition['upload_directory'] . "/",
            '#field_suffix' => ".xml",
            '#default_value' => $configuration['filename'],
        );
    }

    public function importerDisplayFeedInfos(array $configuration) {

        if(empty($configuration)) {return;}

        $file = 'public://' . $this->_definition['upload_directory'] . '/' . $configuration['filename'] . '.xml';

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

    /*
     * We wrappe the native function, because we need our own submit method (automatically derived from the wrapper)
     */
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

    public function generatorListingParamsStep(array &$step_elements, array &$form_state, array $args = array()) {

        $directory = 'public://' . $this->_definition['upload_directory'];

        $dir_handle = opendir($directory);

        $files_options = array();

        while($filename = readdir($dir_handle)) {
            if(is_file($directory . '/' . $filename)) {
                $files_options[$filename] = $filename;
            }
        }

        ksort($files_options);

        $step_elements['information'] = array(
            '#markup' => t("Depending on operation type (sale or rent), fields can vary. In order to be sure to create all needed fields, generator should be launched twice. Once for each case (sale and rent)"),
        );

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
                                "An aC3 virtual type named '@name' and matching the primary type '@primary' already exists.<br />It's mapped on '@type' Drupal's content type.",
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
                                "An aC3 virtual type named '@name' and matching the primary type '@primary' already exists.<br />Since it's not mapped on any Drupal's content type, it can be redefined.",
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

        $filepath = 'public://' . $this->_definition['upload_directory'] . '/' . $wizzard_params['params']['connector']['file'];
        $item_index = $wizzard_params['params']['connector']['item_index'];

        if(!is_file($filepath)) {
            $exc = new SfaImporterException();
            $exc->message = t("File '@file' doesn't exists", array('@file' => $filepath));
            throw $exc;
            return;
        }

        $xml = simplexml_load_file($filepath, null, LIBXML_NOCDATA);
        $all_data = $this->_xml2array($xml, false);
        $data = array_slice($all_data['BIEN'], $item_index, 1);
        $data = $data[0];

        $types_bien_array = $this->get_types_bien_array();
        $this->compute_operation($data);
        $type_bien = $this->compute_type_bien($data);
        $wizzard_params['primary_type'] = $type_bien . '@' . $this->_definition['id'];
        $wizzard_params['virtual_name'] = $types_bien_array[$type_bien];

        $fields_list = array(
            $this->_definition['id'] => array(
                'label' => $this->_definition['label'],
                'type' => 'group',
                'locked' => true,
            ),
            'AFF_ID' => array(
                'label' => t("aC3 reference"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['INFO_GENERALES'][0]['AFF_ID']),
                'locked' => true,
            ),
            'NUM_MANDAT' => array(
                'label' => t("Num. mandat"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['OPERATION'][0]['NUM_MANDAT']),
                'locked' => true,
            ),
            'type_operation' => array(
                'label' => t("Operation type"),
                'type' => 'select',
                'values' => $this->get_operations_array(),
                'group' => $this->_definition['id'],
                'multiple' => false,
                'locked' => true,
            ),
            'type_bien' => array(
                'label' => t("Type of good"),
                'type' => 'select',
                'values' => $types_bien_array,
                'group' => $this->_definition['id'],
                'multiple' => false,
                'locked' => true,
            ),
            'SS_TYPE' => $this->get_field_definition(
                'SS_TYPE',
                array($data['BIEN'][0]['SS_TYPE']),
                $this->_definition['id'],
                true
            ),
            'DATE_CREATION' => array(
                'label' => t("Offer date"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['INFO_GENERALES'][0]['DATE_CREATION']),
                'locked' => true,
            ),
            'DATE_MAJ' => array(
                'label' => t("Update date"),
                'type' => 'textfield',
                'group' => $this->_definition['id'],
                'values' => array($data['INFO_GENERALES'][0]['DATE_MAJ']),
                'locked' => true,
            ),
        );

        unset($data['INFO_GENERALES']);
        unset($data['OPERATION'][0]['NUM_MANDAT']);
        unset($data['BIEN'][0]['SS_TYPE']);
        unset($data['AGENCE']);
        unset($data['PIECES']); // TODO : @see 'custom' SFA field format
        unset($data['INTITULE']);

        //----- COMMENTAIRES
        $fields_list['COMMENTAIRES'] = array(
            'label' => t("Comments"),
            'type' => 'textarea',
            'values' => array_key_exists('COMMENTAIRES', $data) ? array($data['COMMENTAIRES'][0]['FR']) : array(),
            'locked' => true,
        );

        unset($data['COMMENTAIRES']);

        //----- IMAGES
        $fields_list['GROUP_IMAGES'] = array(
            'label' => t("Images"),
            'type' => 'group',
            'locked' => true,
        );

        $fields_list['IMAGES'] = array(
            'label' => t("Images"),
            'type' => 'image',
            'multiple' => true,
            'values' => array_key_exists('IMAGES', $data) && !empty($data['IMAGES']) ? $data['IMAGES'][0]['IMG'] : array(),
            'group' => 'GROUP_IMAGES',
            'locked' => true,
        );

        unset($data['IMAGES']);

        //----- GROUPS AND FIELDS
        foreach($data as $group => $group_data) {
            $fields_list[$group] = array(
                'label' => $group,
                'type' => 'group',
            );

            foreach($group_data[0] as $xml_tag => $field_data) {
                $fields_list[$xml_tag] = $this->get_field_definition($xml_tag, $field_data, $group);
            }
        }

        $wizzard_params['feed_fields'] = $fields_list;
    }

    public function importerGetFeedListing(array $configuration) {

        $filepath = 'public://' . $this->_definition['upload_directory'] . '/' . $configuration['filename'] . '.xml';

        if(!is_file($filepath)) {
            return t("File '@file' doesn't exists", array('@file' => $filepath));
        }

        $xml = simplexml_load_file($filepath, null, LIBXML_NOCDATA);
        $all_data = $this->_xml2array($xml);

        $types_bien_array = $this->get_types_bien_array();

        $listing = array();
        $weight = 0;

        foreach($all_data['BIEN'] as $index => $data) {

            $this->compute_operation($data);
            $type_bien = $this->compute_type_bien($data);
            $aff_id = $data['INFO_GENERALES'][0]['AFF_ID'];

            $listing[$aff_id] = array(
                'id' => $aff_id,
                'primary_type' => $type_bien . '@' . $this->_definition['id'],
                'primary_type_name' => $types_bien_array[$type_bien],
                'date' => $this->compute_datemaj($data),
                'weight' => $weight++,
            );
        }

        return $listing;
    }

    public function importerGetFeedValues($node, $item_id, array &$title, $language, array $configuration, array &$fields, stdClass $feed_definition) {

        $filepath = 'public://' . $this->_definition['upload_directory'] . '/' . $feed_definition->params['connector']['filename'] . '.xml';

        if(!is_file($filepath)) {
            return t("File '@file' doesn't exists", array('@file' => $filepath));
        }

        $xml = simplexml_load_file($filepath, null, LIBXML_NOCDATA);
        $all_data = $this->_xml2array($xml);
        $data = $all_data['BIEN'][$item_id];

        //---- TITLE
        $title = $data['INTITULE'][0][$this->convert_keycode($language)];
        unset($data['INTITULE']);

        //----- DATA COMPUTING + CUSTOM FIELDS
        $fields['type_bien']['values'] = array($this->compute_type_bien($data));
        $fields['type_operation']['values'] = array($this->compute_operation($data));

        //---- COMMENTAIRES
        $fields['COMMENTAIRES']['values'] = array(trim($data['COMMENTAIRES'][0][$this->convert_keycode($language)]));
        unset($data['COMMENTAIRES']);

        //----- IMAGES

        if(array_key_exists('IMAGES', $data) && !empty($data['IMAGES'])) {
            $fields['IMAGES']['values'] = $data['IMAGES'][0]['IMG'];
            unset($data['IMAGES']);
        }

        //----- ADRESSE : conditional : only if VISIBLE == 'true'
        if(array_key_exists('ADRESSE', $data)) {

            if($data['VISIBLE'][0] != 'true') {
                unset($data['ADRESSE']);
            }
        }

        //----- GEOCODING : conditional : only if LONGITUDE and LATITUDE have been unionized but are empty
        if(array_key_exists('LONGITUDE', $fields) && array_key_exists('LATITUDE', $fields)
            && !array_key_exists('LONGITUDE', $data['LOCALISATION'][0]) && !array_key_exists('LATITUDE', $data['LOCALISATION'][0])) { // filtered by _xml2array()

            $coordinates = $this->_geocode(
                $data['LOCALISATION'][0]['VILLE'],
                $data['LOCALISATION'][0]['PAYS'],
                $data['LOCALISATION'][0]['CODE_POSTAL'],
                $data['LOCALISATION'][0]['ADRESSE']
            );

            if($coordinates != false) {
                $fields['LONGITUDE']['values'] = array($coordinates['lng']);
                $fields['LATITUDE']['values'] = array($coordinates['lat']);
            }
        }

        //----- REMAINING FIELDS
        foreach($data as $group => $values) {
            foreach($values[0] as $tag => $value) {
                if(array_key_exists($tag, $fields)) {
                    $fields[$tag]['values'] = array(trim($value));
                }
            }
        }
    }

    /**
     * Compute date from XML date and images date stamp.
     *
     * @param array $data : the first level of '<BIEN />'
     *
     * @return int : timestamp
     */
    protected function compute_datemaj($data) {

        $date_maj = DateTime::createFromFormat('d/m/Y', trim($data['INFO_GENERALES'][0]['DATE_MAJ']))->format('Y-m-d') . ' 00:00:00';

        if(array_key_exists('IMAGES', $data)) {
            foreach($data['IMAGES'][0]['IMG'] as $url) {
                $image_raw_date = preg_replace("#.+DATEMAJ=#", "", trim($url));
                $image_date = DateTime::createFromFormat('d/m/Y-H:i:s', $image_raw_date)->format('Y-m-d H:i:s');

                if($image_date > $date_maj) {
                    $date_maj = $image_date;
                }
            }
        }

        return DateTime::createFromFormat('Y-m-d H:i:s' ,$date_maj)->format('U');
    }

    protected function convert_keycode($keycode) {
        if($keycode == 'en') {
            return 'US';
        } else {
            return strtoupper($keycode);
        }
    }

    /**
     * Returns an SFA field definition array.
     * If field has no definition class, a generic definition is returned.
     *
     * @param string $xml_tag
     * @param array $data : the first level of '<BIEN />'
     * @param string $group : SFA group to add to definition
     *
     * @return array
     */
    protected function get_field_definition($xml_tag, $xml_value, $group = null, $locked = false) {

        $class = 'SoFeedsAgentAC3Field_' . $xml_tag;
        $classes_path = drupal_get_path('module', 'so_feedsagent_connector_ac3') . '/fields_definitions/';
        $class_file = $classes_path . $class . '.class.php';

        require_once $classes_path . 'SoFeedsAgentAC3Field_Interface' . '.class.php';

        if(is_file($class_file)) {
            require_once $class_file;

            $definition = $class::getFieldDefinition();

        } else {

            $definition = array(
                'label' => $xml_tag,
                'type' => 'undefined',
                'values' => array($xml_value),
            );
        }

        if($group != null) {
            $definition['group'] = $group;
        }

        if($locked != false) {
            $definition['locked'] = true;
        }

        return $definition;
    }

    /**
     * Get current BIEN operation.
     * Alter the $data array and replace operation node name by the generic term 'OPERATION'.
     *
     * @param array $data : the first level of '<BIEN />'
     */
    protected function compute_operation(&$data) {

        $operations = $this->get_operations_array();

        foreach($operations as $xml_node_name => $label) {
            if(array_key_exists($xml_node_name, $data)) {
                $data['OPERATION'] = $data[$xml_node_name];
                unset($data[$xml_node_name]);
                return $xml_node_name;
            }
        }
    }

    protected function get_operations_array() {
        return array(
            'VENTE' => t("Sale"),
            'LOCATION' => t("Rent"),
        );
    }

    /**
     * Return the good type system value
     *
     * @param array $data : the first level of '<BIEN />'
     */
    protected function compute_type_bien(&$data) {
        $types_biens = array_keys($this->get_types_bien_array());

        foreach($types_biens as $type_bien) {
            if(array_key_exists($type_bien, $data)) {
                $data['BIEN'] = $data[$type_bien];
                unset($data[$type_bien]);
                return $type_bien;
            }
        }
    }

    protected function get_types_bien_array() {
        return array(
            "APPARTEMENT" => t("Apartment"),
            "MAISON" => t("House"),
            "AGRICOLE_VITICOLE" => t("Agricultural and wine sector"),
            "PARKING" => t("Parking/Box"),
            "PROGRAMME_NEUF" => t("New program"),
            "TERRAIN" => t("Land"),
            "DEMEURE" => t("Luxury property"),
            "FOND_COMMERCE" => t("Leasehold and sale of lease"),
            "FORET" => t("Forest area"),
            "IMMEUBLE" => t("Building"),
            "INCONNU" => t("Others"),
            "LOCAL_PROFESSIONNEL" => t("Office / Business premises, warehouse, industrial premises and walls"),
            "CAVE" => t("Cellar"),
        );
    }

    /**
     *
     * @param simpleXmlElement $xml
     * @param boolean $filter : delete empty entries
     * @return array
     */
    protected function _xml2array(simpleXmlElement $xml, $filter = true)
    {
        $out = array();

        foreach ($xml->children() as $node) {

            if(count($node->children()) == 0) {
                $val = trim(strval($node)); // we have nodes which initialy only contain '\r\n'.

                if(strlen($val) > 0 || $filter == false) {
                    if($node->getName() == 'IMG') {
                        $out[$node->getName()][] = $val;
                    } else {
                        $out[$node->getName()] = $val;
                    }
                }
            } else {
                $sub_out = $this->_xml2array($node, $filter);

                if(!empty($sub_out)) {
                    if($node->getName() == 'BIEN') {
                        $out[$node->getName()][$sub_out['INFO_GENERALES'][0]['AFF_ID']] = $sub_out;
                    } else {
                        $out[$node->getName()][] = $sub_out;
                    }
                }
            }
        }

        return $out;
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
    protected function _geocode($city = "", $country = "", $cp = "", $address1 = "", $address2 = "") {

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
}