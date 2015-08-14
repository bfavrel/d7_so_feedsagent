<?php

function so_feedsagent_date_instance($name, $label, $content_type, $options) {
    
    return array(
        'label' => $label,
        'widget' => array(
            'weight' => $options['weight'],
            'type' => 'date_popup',
            'module' => 'date',
            'active' => 1,
            'settings' => array(
            'input_format' => 'd/m/Y - H:i:s',
            'input_format_custom' => '',
            'year_range' => '-3:+3',
            'increment' => '15',
            'label_position' => 'none',
            'text_parts' => array(),
            'display_all_day' => 0,
            'repeat_collapsed' => 0,
            ),
        ),
        'settings' => array(
            'default_value' => '',
            'default_value_code' => '',
            'default_value2' => 'same',
            'default_value_code2' => '',
            'user_register_form' => FALSE,
        ),
        'display' => array(
            'default' => array(
                'label' => 'hidden',
                'type' => 'date_default',
                'weight' => '0',
                'settings' => array(
                    'format_type' => 'long',
                    'multiple_number' => '',
                    'multiple_from' => '',
                    'multiple_to' => '',
                    'fromto' => 'both',
                    'show_repeat_rule' => 'show',
                ),
                'module' => 'date',
            ),
            'full' => array(
                'label' => 'hidden',
                'type' => 'date_default',
                'weight' => '0',
                'settings' => array(
                    'format_type' => 'long',
                    'multiple_number' => '',
                    'multiple_from' => '',
                    'multiple_to' => '',
                    'fromto' => 'both',
                    'show_repeat_rule' => 'show',
                ),
                'module' => 'date',
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