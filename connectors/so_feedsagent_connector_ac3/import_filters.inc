<?php

/**
 * The field filter callback signature.
 *
 * @param string $raw_value : the aC3's raw values (indexed by feed field tag) as they appear out of the XML parser.
 *                            NOTE : at this time, value has be trimmed.
 * 
 * @param array &$filtered_values : non-associative array of final values.
 *
 * @param array $ac3_data : the aC3's raw values (indexed by feed field tag) as they appear out of the XML parser.
 *                          Useful to get also non unionized fields' values.
  *
 * @param array &$fields : the virtual type fields' array.
 *                         Useful if a filter needs to alter other fields.
 *                         '$filtered_values' (above) can be accessed by $fields['AC3_FIELD_TAG']['filtered_values'] entries.
 *
 * @param object $node : the traditional node object. Here, you can populate foreign fields.
 *
 *
 * @param array $virtual_type_params
 */
function sfa_AC3_FIELD_TAG_field_filter($raw_value, &$filtered_values, $ac3_data, &$fields, $node, $connector_params, $virtual_type_params) {}

/**
 * The field type filter callback signature.
 *
 * Same parameters as above, except for :
 * @param string $ac3_field_tag : the raw field name. Use "$ac3_data[$ac3_field_tag]" to get the current field value.
 *
 * @see so_feedsagent.module:1115:so_feedsagent_get_available_fields_types() for type names.
 */
function sfa_SFA_FIELD_TYPE_type_filter($raw_value, &$filtered_values, $ac3_field_tag, $ac3_data, &$fields, $node, $connector_params, $virtual_type_params) {}

//--------------------------------------------------------------------------------------------------------------------------------

// YOUR CODE HERE