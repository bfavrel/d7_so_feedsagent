<?php

abstract class SoFeedsAgentConnectorAbstract
{
    protected $_definition;

    public function __construct(array $definition) {
        $this->_definition = $definition;
    }

    /**
     * Return the static configuration of the connector type.
     *
     * @return array :
     *          - 'implements' : array : features implemented by the connector : 'generator' and/or 'importer'
     *          - 'dynamic_allowed_values_only' : boolean : no allowed value will be saved at fields creations. Allowed values will be filled by importer, at need.
     */
    public static function getFeatures() {
        return array();
    }

    /**
     * Getter for definition object
     *
     * @param $property : property to return
     *
     * @return mixed/array : be carreful : NOT an object !
     */
    public function getDefinition($property = null) {

        if(empty($property)) {
            return $this->_definition;
        } else {
            return $this->_definition[$property];
        }
    }

    /**
     * Connector's configuration form
     *
     * This method is first called by connectors overview form, in order to know whether or not to provide an 'edit' link.
     * So, in case where $touch_form is set to 'true, if the connector implements a config form, it should only return 'true'.
     * Warning : only the concerned part of the original form is passed in $form.
     * instead of a fully loaded array.
     *
     * @param array &$form
     * @param array &$form_state
     * @param boolean $touch_form
     *
     * @return mixed : array or boolean
     */
    public function connectorConfigurationForm(array &$form, array &$form_state, $touch_form = false) {
        return false;
    }

    /**
     * Connector's configuration form validation
     *
     * @param array $form
     * @param array &$values : = $form_state['values']['configuration']['connector'] : think about it when setting form errors
     */
    public function connectorConfigurationFormValidate(array $form, array &$values) {}

    /**
     * Populates the 'feed_fields' entry of '$form_state[wizzard_params]' with the list of fields to display in fields overview list.
     *
     * In addition it populates the 'virtual_name' (stored in database in {feedsagent_content_types}.virtual_name) and the 'primary_type'
     * (stored in database in {feedsagent_content_types}.primary_type) entries of the wizzard's params.
     * By convention, the 'primary_type' entry must follow this pattern : '[primary_type_id]@[connector_id]'
     *
     * @param array $wizzard_params : the values collected in each form steps, with new additions (see below) when method returns.
     *
     * @return array : indexed by raw field name : this index will be use to define the default Field's name : "field_[connector]_[raw_field_name]_[label]" (sanitized to 32 chars)
     *                                             IMPORTANT : the raw field name of groups' fields will be splitted on double underscore before addition, and only the first part will be used.
     *                      - 'label' : string : human readable field name. If null, the fields name will be used instead.
     *                      - 'type' : string : field type or 'group' (@see so_feedsagent_get_available_fields_types() for valid fields types)
     *                      - 'group' : the index (raw field's name) of a PREVIOUSLY created 'group' type list entry.
     *                      - 'values' : array : raw value => label : authorized values for multiples widgets (radios, etc.) or just an informative value (for 'text', etc.)
     *                      - 'multiple' : boolean : if set to true, force the widget to be multiple.
     *                      - 'locked' : boolean : field can't be choosen or discarded by the user.
     *                      - 'prevent_sorting' : boolean : if set to 'true', field/group will not be sorted in feed fields overview form.
     *                                                      NOTA : locked fields are naturally prevented from sorting and will appear before any other
     *                                                             fields, even non-sorted ones.
     */
    public function generatorPopulateFieldsDefinitionsList(array &$wizzard_params) {
        return array();
    }

    /**
     * Provides forms elements to feed configuration form
     *
     * @param array &$form
     * @param array $configuration
     */
    public function importerFeedConfigurationForm(array &$form, array $configuration) {}

    /**
     * Feed's configuration form validation
     *
     * @param array $form
     * @param array &$values : = $form_state['values']['configuration']['connector'] : think about it when setting form errors
     */
    public function importerFeedConfigurationFormValidate(array $form, array &$values) {}

    /**
     * Provides usefull infos about a feed, to display in overview list.
     *
     * @param array $configuration : the params submited by the connector's form elements
     *
     * return string : HTML to display
     */
    public function importerDisplayFeedInfos(array $configuration) {
        return "";
    }

    /**
     * Returns the list of feed's items.
     *
     * @param array $configuration : the connector's params of the feed
     *
     * @return mixed : - array : indexed by 'id' :
     *                      - 'id' : string : id of the item
     *                      - 'parent_id' : mixed : id of the parent item or null
     *                      - 'primary_type' : string : the item type in feed's context (type@feed_type)
     *                      - 'primary_type_name' : string : human readable of the primary type
     *                      - 'date' : int : item date in timestamp format
     *                      - 'weight' : int : item order in listing
     *
     *                 - boolean : false if no xml has been retrieved.
     */
    public function importerGetFeedListing(array $configuration) {
        return array();
    }

    /**
     * Populate an array of feed items with matching values
     *
     * @param object $node
     * @param string $item_id
     * @param array $title : after processing, title is a string containing the first non null value matching a field's name encountered.
     * @param string $language
     * @param array $configuration : the content type's params set by the connector
     * @param array &$fields : populated with feed's values on return
     * @param stdClass $feed_definition
     *
     * @return mixed : void if no error, a string describing error instead.
     */
    public function importerGetFeedValues($node, $item_id, array &$title, $language, array $configuration, array &$fields, stdClass $feed_definition) {}


    /**
     * Provides an URL to item's data
     *
     * @param string $item_id
     * @param string $language
     * @param stdClass $type_definition
     * @param stdClass $feed_definition
     *
     * @return string
     */
    public function importerGetItemURL($item_id, $language, stdClass $type_definition, stdClass $feed_definition) {
        return false;
    }

    /**
     * Polish raw fields' names, to make them D7/Fields/SQL compatible.
     * TODO : write a security to ensure unicity of a field's name.
     *
     * @param string $prefix : just used to count chars. It won't be added to the final string.
     * @param string $field_name : the raw field's name
     * @param array $existing_fields_names : fields name already used // NOT USED FOR THE MOMENT
     *
     * @return string
     */
    public function sanitizeFieldName($prefix, $field_name, array $existing_fields_names = array()) {

        $remaining_chars = 32 - strlen($prefix); // 32 : limite Drupal

        $field_name = strtolower($field_name);

        if(module_exists('transliteration')) {
            $field_name = transliteration_get($field_name, '_');
        }

        $field_name = preg_replace('#[^0-9a-z_]#', '_', $field_name);

        // on supprime les doubles underscore éventuellement générés
        do {
            $field_name = str_replace("__", "_", $field_name, $count);
        } while ($count > 0);

        // On essaye de préserver l'intégrité de la chaine en supprimant des underscores simples restants
        if(strlen($field_name) > $remaining_chars) {
            $field_name = preg_replace('#_#', '', $field_name);
        }

        // Tant-pis : on coupe
        if(strlen($field_name) > $remaining_chars) {
            $field_name = substr($field_name, 0, $remaining_chars);
        }

        // TODO : $existing_fields_names :
        // Le nom résultant existe déjà  ? On lui ajoute '_{numéro d'ordre}' en prenant garde à la longueur (recursion)

        return $field_name;
    }

    /**
     * Helper : convert a simpleXmlElement in an array well formed.
     * JSON method is messy.
     *
     * @param simpleXmlElement $xml
     *
     * @return array
     */
    protected function _xml2array(simpleXmlElement $xml)
    {
        $arr = array();

        foreach ($xml->children() as $r) {

            if(count($r->children()) == 0) {
                $arr[$r->getName()] = strval($r);
            } else {
                $arr[$r->getName()][] = $this->_xml2array($r);
            }
        }

        return $arr;
    }
}
