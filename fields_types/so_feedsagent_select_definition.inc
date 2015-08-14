<?php

function so_feedsagent_select_definition($name, $options = array()) {
    
    return array(
        'translatable' => '1',
        'entity_types' => array('node'), // seul les content types doivent accéder à ce champ
        'settings' => array(
            'allowed_values' => (array)$options['values'],
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
                        'field_data_field_test_checkboxes' => array(
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
        'type' => 'list_text',
        'module' => 'list',
        'active' => '1',
        'locked' => '0',
        'cardinality' => '1',
        'deleted' => '0',
        'columns' => array(
            'value' => array(
                'type' => 'varchar',
                'length' => 255,
                'not null' => FALSE,
            ),
        ),
        'bundles' => array(
            'node' => array(), // on propose le champ dans tous les content types
        ),
    );
}