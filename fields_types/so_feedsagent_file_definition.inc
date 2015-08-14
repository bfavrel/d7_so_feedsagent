<?php

function so_feedsagent_file_definition($name, $options = array()) {
    
    return array(
        'translatable' => '1',
        'entity_types' => array(),
        'settings' => array(
            'display_field' => 0,
            'display_default' => 0,
            'uri_scheme' => 'public',
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
                        'fid' => $name . '_fid',
                        'display' => $name . '_display',
                        'description' => $name . '_description',
                    ),
                ),
                'FIELD_LOAD_REVISION' => array(
                    'field_revision_' . $name => array(
                        'fid' => $name . '_fid',
                        'display' => $name . '_display',
                        'description' => $name . '_description',
                    ),
                ),
            ),
            ),
        ),
        'foreign keys' => array(
            'fid' => array(
                'table' => 'file_managed',
                'columns' => array(
                    'fid' => 'fid',
                ),
            ),
        ),
        'indexes' => array(
            'fid' => array(
                'fid',
            ),
        ),
        'field_name' => $name,
        'type' => 'file',
        'module' => 'file',
        'active' => '1',
        'locked' => '0',
        'cardinality' => ($options['multiple'] == true ? FIELD_CARDINALITY_UNLIMITED : 1), // multiple = -1
        'deleted' => '0',
        'columns' => array(
            'fid' => array(
                'description' => 'The {file_managed}.fid being referenced in this field.',
                'type' => 'int',
                'not null' => FALSE,
                'unsigned' => TRUE,
            ),
            'display' => array(
                'description' => 'Flag to control whether this file should be displayed when viewing content.',
                'type' => 'int',
                'size' => 'tiny',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'default' => 1,
            ),
            'description' => array(
                'description' => 'A description of the file.',
                'type' => 'text',
                'not null' => FALSE,
            ),
        ),
        'bundles' => array(
            'node' => array(), // on propose le champ dans tous les content types
        ),
    );
}