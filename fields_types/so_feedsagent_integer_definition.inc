<?php

function so_feedsagent_integer_definition($name, $options = array()) {
    
    return array(
        'translatable' => '1',
        'entity_types' => array('node'), // seul les content types doivent accéder à ce champ
        'settings' => array(),
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
        'indexes' => array(),
        'field_name' => $name,
        'type' => 'number_integer',
        'module' => 'number',
        'active' => '1',
        'locked' => '0',
        'cardinality' => ($options['multiple'] == true ? FIELD_CARDINALITY_UNLIMITED : 1), // multiple = -1
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