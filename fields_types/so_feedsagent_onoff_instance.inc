<?php

function so_feedsagent_onoff_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'options_onoff',
            'module' => 'options',
            'active' => 1,
            'settings' => array(
                'display_label' => 1,
            ),
        ),
        'settings' => array(
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => 'hidden',
                'type' => 'i18n_list_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'i18n_field',
            ),
            'full' => array(
                'label' => 'hidden',
                'type' => 'i18n_list_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'i18n_field',
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
        'default_value' => array(
            array(
                'value' => '0',
            ),
        ),
        'field_name' => $name,
        'entity_type' => 'node',
        'bundle' => $content_type,
        'deleted' => '0',
    );
}