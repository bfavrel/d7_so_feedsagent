<?php

function so_feedsagent_textfield_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'text_textfield',
            'module' => 'text',
            'active' => 1,
            'settings' => array(
                'size' => '60',
            ),
        ),
        'settings' => array(
            'text_processing' => '0',
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => $options['multiple'] == true ? 'above' : 'inline',
                'type' => 'text_plain',
                'weight' => '0',
                'settings' => array(),
                'module' => 'text',
            ),
            'full' => array(
                'label' => $options['multiple'] == true ? 'above' : 'inline',
                'type' => 'text_plain',
                'weight' => '0',
                'settings' => array(),
                'module' => 'text',
            ),
            'teaser' => array(
                'type' => 'hidden',
                'label' => 'hidden',
                'settings' => array(),
                'weight' => '0',
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