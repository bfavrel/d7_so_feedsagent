<?php

function so_feedsagent_textfield_definition($name, $options = array()) {

    return array(
        'translatable' => '1',
        'entity_types' => array('node'), // seul les content types doivent accéder à ce champ
        'settings' => array(
            'max_length' => '512',
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
                            'format' => $name . '_format',
                        ),
                    ),
                    'FIELD_LOAD_REVISION' => array(
                        'field_revision_' . $name => array(
                            'value' => $name . '_value',
                            'format' => $name . '_format',
                        ),
                    ),
                ),
            ),
        ),
        'foreign keys' => array(
            'format' => array(
                'table' => 'filter_format',
                'columns' => array(
                    'format' => 'format',
                ),
            ),
        ),
        'indexes' => array(
            'format' => array(
                'format',
            ),
        ),
        'field_name' => $name,
        'type' => 'text',
        'module' => 'text',
        'active' => '1',
        'locked' => '0',
        'cardinality' => ($options['multiple'] == true ? FIELD_CARDINALITY_UNLIMITED : 1), // multiple = -1
        'deleted' => '0',
        'columns' => array(
            'value' => array(
                'type' => 'varchar',
                'length' => '512',
                'not null' => FALSE,
            ),
            'format' => array(
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