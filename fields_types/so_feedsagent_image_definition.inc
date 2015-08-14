<?php

function so_feedsagent_image_definition($name, $options = array()) {
    
    return array(
        'translatable' => '1',
        'entity_types' => array('node'), // seul les content types doivent accéder à ce champ
        'settings' => array(
            'uri_scheme' => 'public',
            'default_image' => $options['default_image'],
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
                            'alt' => $name . '_alt',
                            'title' => $name . '_title',
                            'width' => $name . '_width',
                            'height' => $name . '_height',
                        ),
                    ),
                    'FIELD_LOAD_REVISION' => array(
                        'field_revision_' . $name => array(
                            'fid' => $name . '_fid',
                            'alt' => $name . '_alt',
                            'title' => $name . '_title',
                            'width' => $name . '_width',
                            'height' => $name . '_height',
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
        'type' => 'image',
        'module' => 'image',
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
            'alt' => array(
                'description' => "Alternative image text, for the image's 'alt' attribute.",
                'type' => 'varchar',
                'length' => 512,
                'not null' => FALSE,
            ),
            'title' => array(
                'description' => "Image title text, for the image's 'title' attribute.",
                'type' => 'varchar',
                'length' => 1024,
                'not null' => FALSE,
            ),
            'width' => array(
                'description' => 'The width of the image in pixels.',
                'type' => 'int',
                'unsigned' => TRUE,
            ),
            'height' => array(
                'description' => 'The height of the image in pixels.',
                'type' => 'int',
                'unsigned' => TRUE,
            ),
        ),
        'bundles' => array(
            'node' => array(), // on propose le champ dans tous les content types
        ),
    );
}