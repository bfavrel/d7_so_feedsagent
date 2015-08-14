<?php

function so_feedsagent_select_instance($name, $label, $content_type, $options) {

    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'options_select',
            'module' => 'options',
            'active' => 1,
            'settings' => array(),
        ),
        'settings' => array(
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => 'above',
                'type' => 'i18n_list_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'list',
            ),
            'full' => array(
                'label' => 'above',
                'type' => 'i18n_list_default',
                'weight' => '0',
                'settings' => array(),
                'module' => 'list',
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
                'value' => '_none',
            ),
        ),
        'field_name' => $name,
        'entity_type' => 'node',
        'bundle' => $content_type,
        'deleted' => '0',
    );
}