<?php

function so_feedsagent_fieldgroup($name, $label, $content_type, $weight = 0, $children = array()) {
    
    return (object) array(
        'identifier' => $name . '|node|' . $content_type . '|form',
        'group_name' => $name,
        'entity_type' => 'node',
        'bundle' => $content_type,
        'mode' => 'form',
        'parent_name' => '',        
        'label' => $label,
        'weight' => $weight,
        'children' => $children,
        'format_type' => 'fieldset',
        'format_settings' => array(
            'formatter' => 'collapsed',
            'instance_settings' => array(
                'description' => '',
                'classes' => '',
                'required_fields' => 0,
            ),
        ),        
    );
}