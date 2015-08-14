<?php

function so_feedsagent_field_info() {

    return array(
        'sfa_type' => array(
            'label' => t("FeedsAgent"),
            'description' => t("Display FeedsAgent data elements."),
            'default_widget' => 'sfa_couplings_widget',
            'default_formatter' => 'sfa_default_formatter',
            'instance_settings' => array(),
        ),
    );
}

function so_feedsagent_field_instance_settings_form($field, $instance) {
    return array();
}

//------------------------------------------------------------------------------

function so_feedsagent_field_widget_info() {

    return array(
        'sfa_couplings_widget' => array(
            'label' => t("Coupled items"),
            'field types' => array('sfa_type'),
            'description' => t("Display associated items."),
            'behaviors' => array(
                'multiple values' => FIELD_BEHAVIOR_DEFAULT,
                'default value' => FIELD_BEHAVIOR_DEFAULT,
            ),
            'settings' => array(),
        ),
    );
}

function so_feedsagent_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

    if($instance['widget']['type'] != 'sfa_couplings_widget') {return $element;}

    $coupling = so_feedsagent_get_coupled_items_by_nid($element['#entity']->nid, 'auto');
    if(empty($coupling)) {return $element;}

    $coupling_element = array();

    if($coupling['type'] == 'parent') {

        $parent_nid = key($coupling['sfa_contents']);
        $parent_title = db_query("SELECT title FROM {node} WHERE nid=?", array($parent_nid))->fetchColumn();

        $coupling_element = array(
            '#theme' => 'item_list',
            '#title' => t("This item is a child of this item"),
            '#type' => 'ul',
            '#items' => array(l($parent_title, drupal_get_path_alias('node/' . $parent_nid), array('attributes' => array('target' => '_blank')))),
        );

    } elseif($coupling['type'] == 'childs') {

        $coupling_element_items = array();

        foreach($coupling['sfa_contents'] as $child_nid => $child) {
            $child_title = db_query("SELECT title FROM {node} WHERE nid=?", array($child_nid))->fetchColumn();
            $coupling_element_items[] = l($child_title, drupal_get_path_alias('node/' . $child_nid), array('attributes' => array('target' => '_blank')));
        }

        $coupling_element = array(
            '#theme' => 'item_list',
            '#title' => t("This item is the parent of these items"),
            '#type' => 'ul',
            '#items' => $coupling_element_items,
        );
    }

    $element += array(
        'couplages' => array(
            '#type' => 'fieldset',
            '#title' => $instance['label'],
            '#collapsible' => true,
            '#collapsed' => true,

            'couplages' => $coupling_element,
        ),
    );

    return $element;
}

//------------------------------------------------------------------------------

function so_feedsagent_field_formatter_info() {

    return array(
        'sfa_default_formatter' => array(
            'label' => t("Default"),
            'field types' => array('sfa_type'),
            'settings' => array(
                'view_mode_display' => 'teaser',
                'label_for_parent' => "Parent",
                'label_for_childs' => "Childs",
            ),
        ),
    );
}

function so_feedsagent_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {

    if($instance['widget']['type'] != 'sfa_couplings_widget') {return;}

    $available_view_modes = array_flip(array_keys($instance['display']));
    $entity_info = entity_get_info('node');
    $view_modes = $entity_info['view modes'];
    $view_modes_options = array_intersect_key($view_modes, $available_view_modes);

    array_walk($view_modes_options, function(&$val) {
        $val = $val['label'];
    });

    return array(
        'label_for_parent' => array(
            '#type' => 'textfield',
            '#title' => t("Label for parent"),
            '#description' => t("In default language"),
            '#default_value' => $instance['display'][$view_mode]['settings']['label_for_parent'],
        ),

        'label_for_childs' => array(
            '#type' => 'textfield',
            '#title' => t("Label for childs"),
            '#description' => t("In default language"),
            '#default_value' => $instance['display'][$view_mode]['settings']['label_for_childs'],
        ),

        'view_mode_display' => array(
            '#type' => 'select',
            '#title' => t("View mode used to display items"),
            '#options' => $view_modes_options,
            '#default_value' => $instance['display'][$view_mode]['settings']['view_mode_display'],
        ),
    );
}

function so_feedsagent_field_formatter_settings_summary($field, $instance, $view_mode) {

    if($instance['widget']['type'] != 'sfa_couplings_widget') {return;}

    $entity_info = entity_get_info('node');
    $view_modes = $entity_info['view modes'];

    $summary = "<strong>" . t("View mode") . " : </strong>" . $view_modes[$instance['display'][$view_mode]['settings']['view_mode_display']]['label'];

    if(array_key_exists('label_for_parent', $instance['display'][$view_mode]['settings'])
            && !empty($instance['display'][$view_mode]['settings']['label_for_parent'])) {

        $label_for_parent = i18n_string(
            'field:' . $field['field_name'] . ':' . $instance['bundle'] . ':label_for_parent',
            $instance['display'][$view_mode]['settings']['label_for_parent'],
            array('update' => true)
        );

        $summary .= "<br />" . "<strong>" . t("Label for parent") . " : </strong>" . $label_for_parent;
    }

    if(array_key_exists('label_for_childs', $instance['display'][$view_mode]['settings'])
            && !empty($instance['display'][$view_mode]['settings']['label_for_childs'])) {

        $label_for_childs = i18n_string(
            'field:' . $field['field_name'] . ':' . $instance['bundle'] . ':label_for_childs',
            $instance['display'][$view_mode]['settings']['label_for_childs'],
            array('update' => true)
        );

        $summary .= "<br />" . "<strong>" . t("Label for childs") . " : </strong>" . $label_for_childs;
    }

    return $summary;
}

function so_feedsagent_field_formatter_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items, $displays) {

    foreach($instances as $nid => $instance) {

        if($instance['widget']['type'] != 'sfa_couplings_widget') {continue;}

        $coupling = so_feedsagent_get_coupled_items_by_nid($nid, 'auto');
        if(empty($coupling)) {continue;}

        $items[$nid][] = $coupling;
    }
}

function so_feedsagent_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {

    if($instance['widget']['type'] != 'sfa_couplings_widget') {return;}

    $elements = array();

    foreach($items as $item) {

        if($item['type'] == 'parent') {

            $coupling_title = i18n_string(
                'field:' . $field['field_name'] . ':' . $entity->type . ':label_for_parent',
                $display['settings']['label_for_parent']
            );

            $node = node_load(key($item['sfa_contents']));
            $node_view = node_view($node, $display['settings']['view_mode_display']);

            $coupling_items[] = render($node_view);

        } elseif($item['type'] == 'childs') {

            $coupling_title = i18n_string(
                'field:' . $field['field_name'] . ':' . $entity->type . ':label_for_childs',
                $display['settings']['label_for_childs']
            );

            $coupling_items = array();

            foreach($item['sfa_contents'] AS $nid => $sfa_content) {

                $node = node_load($nid);
                $node_view = node_view($node, $display['settings']['view_mode_display']);

                $coupling_items[] = render($node_view);
            }
        }

        $elements[] = array(
            'sfa_coupling' => array(
                '#theme' => 'item_list',
                '#title' => $coupling_title,
                '#type' => 'ul',
                '#attributes' => array('class' => array('sfa_coupling_items')),
                '#items' => $coupling_items,
            ),
        );
    }

    return $elements;
}

//------------------------------------------------------------------------------

function so_feedsagent_field_is_empty($item, $field) {
    return false;
}
