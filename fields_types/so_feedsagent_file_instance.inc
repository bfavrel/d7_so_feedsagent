<?php

function so_feedsagent_file_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'file_generic',
            'module' => 'file',
            'active' => 1,
            'settings' => array(
                'progress_indicator' => 'throbber',
            ),
        ),
        'settings' => array(
            'file_directory' => $options['file_folder'],
            'file_extensions' => 'txt doc pdf xls',
            'max_filesize' => '',
            'description_field' => 0,
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => 'above',
                'type' => 'file_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'file',
            ),
            'full' => array(
                'label' => 'above',
                'type' => 'file_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'file',
            ),
            'teaser' => array(
                'label' => 'hidden',
                'type' => 'hidden',
                'weight' => '0',
                'settings' => array(),
            ),
        ),
        'required' => 0,
        'description' => '',
        'field_name' => $name,
        'entity_type' => 'node',
        'bundle' => $content_type,
        'deleted' => '0',
    );
}