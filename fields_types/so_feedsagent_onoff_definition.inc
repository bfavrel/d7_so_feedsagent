<?php

function so_feedsagent_onoff_definition($name, $options = array()) {

    $allowed_values = array(0, 1);

    if(!empty($options['values'])) {
        $allowed_values = array('', $options['values'][0]);
    } elseif(!empty($options['label'])) {
        $allowed_values = array('', $options['label']);
    }

    return array(
        'translatable' => '1',
        'entity_types' => array(),
        'settings' => array(
            'allowed_values' =>  $allowed_values,
            'allowed_values_function' => '',
        ),
        'storage' => array(
            'type' => 'field_sql_storage',
            'settings' => array(),
            'module' => 'field_sql_storage',
            'active' => '1',
            'details' => array(
            'sql' => array(
                'FIELD_LOAD_CURRENT' => array(
                    'field_data_' . $name => array(
                        'value' => $name . '_value',
                    ),
                ),
                'FIELD_LOAD_REVISION' => array(
                    'field_revision_' . $name => array(
                        'value' => $name . '_value',
                    ),
                ),
            ),
            ),
        ),
        'foreign keys' => array(),
        'indexes' => array(
            'value' => array(
                'value',
            ),
        ),
        'field_name' => $name,
        'type' => 'list_boolean',
        'module' => 'list',
        'active' => '1',
        'locked' => '0',
        'cardinality' => '1',
        'deleted' => '0',
        'columns' => array(
            'value' => array(
                'type' => 'int',
                'not null' => FALSE,
            ),
        ),
        'bundles' => array(
            'node' => array(), // on propose le champ dans tous les content types
        ),
    );
}