<?php

function so_feedsagent_decimal_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'number',
            'module' => 'number',
            'active' => 1,
            'settings' => array(),
        ),
        'settings' => array(
            'min' => '',
            'max' => '',
            'prefix' => '',
            'suffix' => '',
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => $options['multiple'] == true ? 'above' : 'inline',
                'type' => 'number_unformatted',
                'weight' => '0',
                'settings' => array(),
                'module' => 'number',
            ),
            'full' => array(
                'label' => $options['multiple'] == true ? 'above' : 'inline',
                'type' => 'number_unformatted',
                'weight' => '0',
                'settings' => array(),
                'module' => 'number',
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
        'default_value' => NULL,
        'field_name' => $name,
        'entity_type' => 'node',
        'bundle' => $content_type,
        'deleted' => '0',
    );
}