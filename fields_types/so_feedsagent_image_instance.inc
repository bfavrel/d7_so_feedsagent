<?php

function so_feedsagent_image_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'image_image',
            'module' => 'image',
            'active' => 1,
            'settings' => array(
                'progress_indicator' => 'throbber',
                'preview_image_style' => 'thumbnail',
            ),
        ),
        'settings' => array(
            'file_directory' => $options['image_folder'],
            'file_extensions' => 'png gif jpg jpeg',
            'max_filesize' => '',
            'max_resolution' => '',
            'min_resolution' => '',
            'alt_field' => 1,
            'title_field' => 1,
            'default_image' => $options['default_image'],
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => 'hidden',
                'type' => 'image',
                'weight' => '0',
                'settings' => array(
                    'image_style' => '',
                    'image_link' => '',
                ),
                'module' => 'image',
            ),
            'full' => array(
                'label' => 'hidden',
                'type' => 'image',
                'weight' => '0',
                'settings' => array(
                    'image_style' => '',
                    'image_link' => '',
                ),
                'module' => 'image',
            ),
            'teaser' => array(
                'label' => 'hidden',
                'type' => 'image',
                'weight' => '0',
                'settings' => array(
                    'image_style' => '',
                    'image_link' => '',
                ),
                'module' => 'image',
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