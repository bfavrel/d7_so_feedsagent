<?php

function so_feedsagent_textarea_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'text_textarea',
            'module' => 'text',
            'active' => 1,
            'settings' => array(
                'rows' => '5',
            ),
        ),
        'settings' => array(
            'text_processing' => '1',
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => 'hidden',
                'type' => 'text_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'text',
            ),
            'full' => array(
                'label' => 'hidden',
                'type' => 'text_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'text',
            ),
            'teaser' => array(
                'label' => 'hidden',
                'type' => 'hidden',
                'weight' => '0',
                'settings' => array(),
                'module' => 'text',
            ),
        ),
        'required' => 0,
        'description' => '',
        'default_value' => NULL,
        'field_name' => $name,
        'entity_type' => 'node',
        'bundle' => $content_type,
        'deleted' => '0',
    );
}