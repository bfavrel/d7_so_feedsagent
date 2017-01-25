<?php

function so_feedsagent_generator_form($form, &$form_state) {

    drupal_set_title(t("FeedsAgent content type wizard"));

    if(!array_key_exists('step', $form_state)) {

        $form_state['step'] = 0;
        $form_state['previous_step'] = 0;
        $form_state['storage'] = array();

        $form_state['wizzard_params'] = array(
            'operation' => 'init',
        );

        $form_state['form_map'] = array(
            'init' => array(
                'label' => null,
                'steps' => array(
                    0 => array(
                        'title' => t("Operations"),
                        'function' => 'so_feedsagent_generator_initial_step',
                    ),
                ),
            ),
        );
    }

    $step_elements = array();
    $form_additions = array();

    while(empty($step_elements)) {
        $form_additions = (array)so_feedsagent_generator_step_callback($step_elements, $form_state);

        if(empty($step_elements)) {
            so_feedsagent_generator_navigation_submit(array(), $form_state);
        }
    }

    $step_title = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['title'];

    $step_subtitle = array();
    $step_subtitle[] = !empty($form_state['wizzard_params']['type']) ? t("Content type : '@type'", array('@type' => $form_state['wizzard_params']['type'])) : "";
    $step_subtitle[] = !empty($form_state['wizzard_params']['connector']) ? t("Connector : '@connector'", array('@connector' => $form_state['wizzard_params']['connector'])) : "";
    $step_subtitle[] = !empty($form_state['wizzard_params']['virtual_name']) ? t("Virtual name : '@virtual_name'", array('@virtual_name' => $form_state['wizzard_params']['virtual_name'])) : "";
    $step_subtitle[] = !empty($form_state['wizzard_params']['primary_type']) ? t("Primary type : '@primary_type'", array('@primary_type' => $form_state['wizzard_params']['primary_type'])) : "";
    $step_subtitle = implode(" - ", array_filter($step_subtitle));

    $help = array();
    $step_help = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['help'];

    if(!empty($step_help)) {
        $help = array(
            '#prefix' => "<div class='feedsagent_wizzard_help'>",
            '#markup' => $step_help,
            '#suffix' => "</div>",
        );
    }

    $form = array(
        '#id' => 'so-feedsagent-generator-form',
        '#tree' => true,
        '#attached' => array(
            'css' => array(drupal_get_path('module', 'so_feedsagent') . '/css/so_feedsagent_admin.css'),
            'js' => array(drupal_get_path('module', 'so_feedsagent') . '/scripts/so_feedsagent_admin.js'),
        ),
        '#attributes' => array(
            'class' => array('feedsagent_wizzard'),
        ),

        'step_title' => array(
            '#prefix' => "<h2 class='feedsagent_wizzard_title'>",
            '#markup' => $step_title . "<br /><span>" . $step_subtitle . "</span>",
            '#suffix' => "</h2>",
        ),

        'help' => $help,

        'step_elements' => $step_elements,

        'navigation' => array(
            '#type' => 'actions',
        ),
    );

    $form = array_merge_recursive($form, $form_additions);

    if(array_key_exists('previous_button', $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']])
            && $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['previous_button'] == true
                && $form_state['step'] < count($form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'])) {

        $form['navigation']['previous'] = array(
            '#type' => 'submit',
            '#value' => t("Previous"),
            '#limit_validation_errors' => array(),
            '#submit' => array('so_feedsagent_generator_navigation_submit'),
            '#weight' => -10,
//            '#ajax' => array(
//                'callback' => 'so_feedsagent_generator_wizzard_ajax_callback',
//                'wrapper' => 'so-feedsagent-generator-form',
//                'effect' => 'fade',
//                'event' => 'click',
//                'method' => 'replace',
//                'progress' => array(
//                    'type' => 'throbber',
//                    'message' => null,
//                ),
//            ),
        );
    }

    $steps_num = count($form_state['form_map'][$form_state['wizzard_params']['operation']]['steps']);

    if($form_state['step'] != ($steps_num)) {

        $form['navigation']['next'] = array(
            '#type' => 'submit',
            '#value' => $form_state['step'] == ($steps_num - 1) && $form_state['step'] != 0 ? t("Finish") : t("Next"),
            '#submit' => array('so_feedsagent_generator_navigation_submit'),
            '#weight' => 10,
//            '#ajax' => array(
//                'callback' => 'so_feedsagent_generator_wizzard_ajax_callback',
//                'wrapper' => 'so-feedsagent-generator-form',
//                'effect' => 'fade',
//                'event' => 'click',
//                'method' => 'replace',
//                'progress' => array(
//                    'type' => 'throbber',
//                    'message' => null,
//                ),
//            ),
        );
    }

    return $form;
}

function so_feedsagent_generator_step_callback(&$step_elements, &$form_state) {

    $form_additions = array();

    if(array_key_exists('function', (array)$form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']])) {

        $function = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['function'];
        $form_additions += (array)$function(
            $step_elements,
            $form_state,
            $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['args']
        );

    } elseif(array_key_exists('method', (array)$form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']])) {

        $method = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['method'];
        $connector = so_feedsagent_load_connector($form_state['wizzard_params']['connector']);

        $form_additions += (array)$connector->$method(
            $step_elements,
            $form_state,
            $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['args']
        );

    }

    return $form_additions;
}

function so_feedsagent_generator_wizzard_ajax_callback($form, &$form_state) {
    return $form;
}

function so_feedsagent_generator_navigation_submit($form, &$form_state) {

    $form_state['rebuild'] = true;

    // on stocke l'ensemble des valeurs utilisateur générées à cette étape.
    // '#limit_validation_errors' dépeuple [values], on prend donc les entrées utilisateur à partir de [input]
    // Les valeurs de [input] ne sont pas dans le même ordre que celles qui ont été placées dans [storage] :
    // [storage] reflète [value] qui est ordonné par rapport à $form, tandis que [input] est ordonné par rapport à $_POST.
    if($form_state['triggering_element']['#limit_validation_errors'] === false) {

        // BUG 1 : problème de l'obsolescence : si, par exemple un champ de Feed est sélectionné, suivi d'un retour arrière, la valeur est bien
        // stockées dans le storage, mais pas processed par le submit (normal...). Seulement, la comparaison au moment du submit ne détecte pas de
        // changement, dans la mesure où ce qui a été stocké puis rétabli est identique.
        // --> le système complet d'obsolescence est à revoir !

        // on vérifie si les entrée utilisateur ont changé, par rapport à des éventuelles données précédement stockées
        if(serialize((array)$form_state['storage'][$form_state['step']]) != serialize($form_state['input']['step_elements'])) {

            // on stocke l'ensemble des valeurs utilisateur générées à cette étape
            $form_state['storage'][$form_state['step']] = $form_state['input']['step_elements'];

            $obsolescence = true;
        } else {
            $obsolescence = false;
        }

        if(so_feedsagent_generator_step_process_callbacks($form_state, $obsolescence) == false) {return;}

        // on stocke dans le container de params les valeurs résiduelles (non traitées par une fonction/méthode de soumission).
        // Là, on utilise [values], car les valeurs de type '#value' ne sont pas présentes dans [input].
        $form_state['wizzard_params'] = array_merge((array)$form_state['wizzard_params'], (array)$form_state['values']['step_elements']);

    } else {
        // BUG 2 : lors d'un retour à partir de l'étape de définition des champs, les valeurs ne sont pas passé : ce sont
        // celles du form par défaut qui peuplent [input]. En revanche, ça fonctionne pour l'étape de sélection des champs du feed.
        // --> Voir BUG 1 ci-dessus

        // BUG 3 : pour les types imagefield, la valeur soumise dans [input] est différente de celle de [values] :
        // dans le second cas, 'fid' est un integer, et dans le premier, une entrée du array [default] (avec 'upload' comme seconde entrée)
        // Il n'a pas encore été déterminé si le problème venait de Field ou du module.

        //$form_state['storage'][$form_state['step']] = $form_state['input']['step_elements'];
        // --> solution provisoire au BUG 1

        // A ETUDIER : si on merge les valeurs de [values] avec celle de [input], cela résoud le problème des imagefield et des valeurs non-transmises
        // lors d'un retour arrière.
    }

    $form_state['previous_step'] = $form_state['step'];

    if($form_state['triggering_element']['#parents'][count($form_state['triggering_element']['#parents']) - 1] == 'next') {
        $form_state['step']++;
    } elseif($form_state['triggering_element']['#parents'][count($form_state['triggering_element']['#parents']) - 1] == 'previous') {
        $form_state['step']--;
    }

    // on réinitialise l'élément avec les éventuelles valeurs soumises précédemment pour l'étape en cours
    $form_state['values']['step_elements'] = (array)$form_state['storage'][$form_state['step']];
}

/**
 * Step's validation/submission
 *
 * @param array $form_state
 * @param boolean $obsolescence : have values changed since last recording ?
 *
 * @return boolean
 */
function so_feedsagent_generator_step_process_callbacks(&$form_state, $obsolescence) {

    if(array_key_exists('function', (array)$form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']])) {

        $validation_function = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['function'] . '_validation';
        $validation = is_callable($validation_function) ? $validation_function($form_state, $obsolescence) : true;

        $submission_function = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['function'] . '_submission';

        if(is_callable($submission_function) && $validation !== false) {

            $debug = array();// HACK :

            $validation = $submission_function($form_state, $obsolescence, $debug);// HACK : $debug

            if(!empty($debug)) {// HACK :
                $form_state = $debug;
            }
        }

    } elseif(array_key_exists('method', (array)$form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']])
            && array_key_exists('connector', $form_state['wizzard_params'])) {

        $connector = so_feedsagent_load_connector($form_state['wizzard_params']['connector']);

        $validation_method = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['method'] . 'Validation';
        $validation = is_callable(array($connector, $validation_method)) ? $connector->$validation_method($form_state, $obsolescence) : true;

        $submission_method = $form_state['form_map'][$form_state['wizzard_params']['operation']]['steps'][$form_state['step']]['method'] . 'Submission';

        if(is_callable(array($connector, $submission_method)) && $validation !== false) {
            $validation = $connector->$submission_method($form_state, $obsolescence);
        }
    }

    return $validation !== false;
}

/**
 * Register a parameter's dependencies
 *
 * @param array &$form_state
 * @param string $parameter
 * @param array $parents
 */
function _so_feedsagent_register_dependency(&$form_state, $parameter, $parents = array()) {

    if(!array_key_exists($parameter, $form_state['wizzard_params']['#dependencies']['parents'])) {
        $form_state['wizzard_params']['#dependencies']['parents'][$parameter] = array();
    } else {
        return; // le parametre a déjà été enregistré
    }

    foreach($parents as $parent) {
        $form_state['wizzard_params']['#dependencies']['parents'][$parent][$parameter] = true; // invalide par défaut
    }

    if(!array_key_exists($parameter, $form_state['wizzard_params']['#dependencies']['children'])) {
        $form_state['wizzard_params']['#dependencies']['children'][$parameter] = $parents;
    }
}

/**
 * Invalidates all dependent parameters.
 *
 * @param array &$form_state
 * @param array $parameters : one or more parent parameters
 */
function _so_feedsagent_invalidate_dependencies(&$form_state, $parameters) {

    foreach($parameters as $parameter) {
        if(array_key_exists($parameter, $form_state['wizzard_params']['#dependencies']['parents']) && !empty($form_state['wizzard_params']['#dependencies']['parents'][$parameter])) {
            $form_state['wizzard_params']['#dependencies']['parents'][$parameter] = array_fill_keys(array_keys($form_state['wizzard_params']['#dependencies']['parents'][$parameter]), true);
        }
    }
}

/**
 * Informs a parameters if it has been invalidated or not.
 *
 * @param array &$form_state
 * @param string $parameter : child parameter
 */
function _so_feedsagent_get_dependency_state(&$form_state, $parameter) {

    $invalidated = false;

    foreach($form_state['wizzard_params']['#dependencies']['children'][$parameter] as $parent) {

        $invalidated |= $form_state['wizzard_params']['#dependencies']['parents'][$parent][$parameter];
    }

    return $invalidated;
}

/**
 * Reset state of an invalid parameter.
 *
 * @param array &$form_state
 * @param string $parameter : child parameter
 */
function _so_feedsagent_reset_dependency_state(&$form_state, $parameter) {

    foreach($form_state['wizzard_params']['#dependencies']['children'][$parameter] as $parent) {

        $form_state['wizzard_params']['#dependencies']['parents'][$parent][$parameter] = false;
    }
}

/**
 * The initial step.
 * No arguments passed : it's a static step.
 *
 * @param array $step_elements
 * @param array $form_state
 * @param array $args : unused
 */
function so_feedsagent_generator_initial_step(&$step_elements, &$form_state, $args = array()) {

    $connector_definitions = so_feedsagent_get_connectors_definitions();

    if(empty($connector_definitions)) {

        drupal_set_message(
            t(
                "At least a connector must be created first : <br />!link",
                array(
                    '!link' => l(t("create a connector"), 'admin/config/so_feedsagent/settings/connectors'),
                )
            ),
            'warning'
        );

        return;
    }

    $form_state['wizzard_params'] += array(
        '#dependencies' => array(
            'parents' => array(),
            'children' => array(),
        ),
        'id' => null,
        'type' => null,
        'connector' => null,
        'virtual_name' => null,
        'ignored' => 0,
        'primary_type' => null,
        'fields' => array(),
        'groups' => array(),
        'params' => array(
            'common' => array(),
            'connector' => array(),
        ),
        'custom_selection' => array(),
        'custom_selection_label' => "",
    );

    foreach($connector_definitions as $infos) {

        $connector_form_map = array();
        $class = $infos->class;
        $module = $infos->module;

        $connector_features = $class::getFeatures();

        if(!in_array('generator', $connector_features['implements'])) {
            continue;
        }

        $function = $module . '_sfa_generator_form_map';
        $connector_form_map = $function();

        $form_state['form_map'] = array_merge($form_state['form_map'], $connector_form_map);
    }

    $options = array();

    foreach($form_state['form_map'] as $operation => $infos) {
        $options[$operation] = $infos['label'];
    }

    unset($options['init']);

    $step_elements = array(
        'operation' => array(
            '#type' => 'select',
            '#title' => "",
            '#options' => $options,
        ),
    );
}

/**
 * The virtual type's basic parameters step.
 *
 * @param array $step_elements
 * @param array $form_state
 * @param array $args : array :
 *                  - 'needs' : non associative array of arrays whith entries :
 *                          - 'param' : string : name of param needed -> '_so_feedsagent_generator_[param]_element()'
 *                             (either 'connector', 'virtual_type', 'primary_type', or 'drupal_type')
 *                          - 'args' : array : arguments to pass to the param's element function.
 */
function so_feedsagent_generator_virtual_type_params_step(&$step_elements, &$form_state, $args = array()) {

    foreach($args['needs'] as $need) {
        $function = '_so_feedsagent_generator_' . $need['param'] . '_element';

        $return = $function($form_state, $need['args']);

        if(!empty($return)) {
            $step_elements[$need['param']] = $return;
            $step_elements[$need['param']]['#weight'] = $need['weight'];
        }
    }
}

function so_feedsagent_generator_virtual_type_params_step_submission(&$form_state, $obsolescence) {

    if($obsolescence == false) {return;}

    // on mappe le nom du paramètre dans le contexte du formulaire, sur le nom réel du paramètre (le nom de sa colonne dans la table
    // et donc également dans les paramètres du wizzard)
    $mapping = array(
        'connector' => 'connector',
        'virtual_type' => 'virtual_type',
        'primary_type' => 'primary_type',
        'drupal_type' => 'type', // le seul paramètre réellement concerné par ce mapping (raison : clarté du nom de fonction)
    );

    foreach($form_state['values']['step_elements'] as $step_element => $value) {

        _so_feedsagent_register_dependency($form_state, $mapping[$step_element]);

        if(array_key_exists($mapping[$step_element], $form_state['wizzard_params']) && $form_state['wizzard_params'][$mapping[$step_element]] != $value) {
            _so_feedsagent_invalidate_dependencies($form_state, array($mapping[$step_element]));
            $form_state['wizzard_params'][$mapping[$step_element]] = $value;
        }
    }

    // ATTENTION ! les connecteurs qui wrappent l'appel à cette __FUNCTION__ doivent prendre soin de de sauvegarder
    // les valeurs avant de lancer l'appel, sinon, elles n'auront plus rien à traiter.
    $form_state['values']['step_elements'] = array();
}

function _so_feedsagent_generator_connector_element(&$form_state, $args = array()) {

    $args += array(
        'id' => null,
        'type' => null,
        'label' => t("Connector"),
    );

    $options = array();
    $connectors_definitions = so_feedsagent_get_connectors_definitions($args['id'], $args['type']);

    foreach($connectors_definitions as $connector_definition) {
        $options[$connector_definition->id] = $connector_definition->label;
    }

    if(count($options) == 1) {
        $form_state['wizzard_params']['connector'] = $connector_definition->id;
        _so_feedsagent_invalidate_dependencies($form_state, array('connector'));
        return; // on skippe l'étape
    }

    asort($options);

    return array(
        '#type' => 'select',
        '#title' => $args['label'],
        '#options' => $options,
        '#default_value' => $form_state['values']['step_elements']['connector'],
    );
}

function _so_feedsagent_generator_primary_type_element(&$form_state, $args = array()) {

    $args += array(
        'id' => null,
        'connector' => null,
        'primary_type' => null,
        'orphans' => true,
        'filter' => null, // null ou 'orphans_only'
        'index' => 'primary_type',
        'label' => t("Primary type"),
        'null_option' => true,
        'null_label' => "- " . t("none") . " -",
    );

    $content_types = so_feedsagent_get_content_types_definitions(
        empty($args['id']) ? $form_state['wizzard_params']['id'] : $args['id'],
        empty($args['connector']) ? $form_state['wizzard_params']['connector'] : $args['connector'],
        empty($args['primary_type']) ? $form_state['wizzard_params']['primary_type'] : $args['primary_type'],
        $args['orphans'],
        $args['index']
    );

    $options = array();

    if(is_object($content_types)) {
        $content_types = array($content_types);
    }

    foreach($content_types as $index => $infos) {

        if($args['filter'] == 'orphans_only') {
            if(empty($infos->type)) {
                $options[$infos->primary_type] = $infos->virtual_name;
            }
        } else {
            $options[$infos->primary_type] = $infos->virtual_name;
        }
    }

    asort($options);

    if($args['null_option'] == true) {
        $options = array('' => $args['null_label']) + $options;
    }

    $default = $form_state['wizzard_params']['primary_type'];

    if(empty($default)) {
        $default = array_key_exists('primary_type', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['primary_type'] : $default;
    }

    return array(
        '#type' => 'select',
        '#title' => $args['label'],
        '#options' => $options,
        '#default_value' => $default,
    );
}

function _so_feedsagent_generator_drupal_type_element(&$form_state, $args = array()) {

    $args += array(
        'id' => null,
        'connector' => null,
        'primary_type' => null,
        'filter' => null, // null ou 'sfa_only'
        'index' => 'id',
        'label' => t("Drupal type"),
        'null_option' => true,
        'null_label' => "- " . t("none") . " -",
    );

    $options = array();
    $nodes_types = node_type_get_names();

    if($args['filter'] == 'sfa_only') {
        $content_types = (array)so_feedsagent_get_content_types_definitions(
            !empty($args['id']) ? $form_state['wizzard_params']['id'] : $args['id'],
            !empty($args['connector']) ? $form_state['wizzard_params']['connector'] : $args['connector'],
            !empty($args['primary_type']) ? $form_state['wizzard_params']['primary_type'] : $args['primary_type'],
            false,
            $args['index']
        );

        foreach($content_types as $content_type => $infos) {
            if(!empty($infos->type)) {
                $options[$content_type] = $nodes_types[$content_type] . " (" . $content_type . ")";
            }
        }

    } else {

        $options = $nodes_types;

        array_walk($options, function(&$val, $key){
            $val .= " (" . $key . ")";
        });
    }

    asort($options);

    if($args['null_option'] == true) {
        $options = array('' => $args['null_label']) + $options;
    }

    $default = $form_state['wizzard_params']['type'];

    if(empty($default)) {
        $default = array_key_exists('type', $form_state['values']['step_elements']) ? $form_state['values']['step_elements']['type'] : $default;
    }

    return array(
        '#type' => 'select',
        '#title' => $args['label'],
        '#options' => $options,
        '#default_value' => $default,
    );
}

function so_feedsagent_generator_content_type_params_step(&$step_elements, &$form_state, $args = array()) {

    if(!empty($form_state['wizzard_params']['type']) && !array_key_exists('content_type_name', $form_state['wizzard_params'])) {
        $step_elements = array(
            '#markup' => t("The present virtual type will be mapped on '@type' type.", array('@type' => $form_state['wizzard_params']['type'])),
        );
        return;
    }

    _so_feedsagent_register_dependency($form_state, 'content_type_params', array('type', 'primary_type'));

    if(_so_feedsagent_get_dependency_state($form_state, 'content_type_params') == true) {
        $form_state['values']['step_elements'] = array();
        _so_feedsagent_reset_dependency_state($form_state, 'content_type_params');
    }

    $name = "";

    if(array_key_exists('content_type_name', $form_state['values']['step_elements'])) {
        $name = $form_state['values']['step_elements']['content_type_name'];
    } elseif(array_key_exists('virtual_name', $form_state['wizzard_params'])) {
        $name = $form_state['wizzard_params']['virtual_name'];
    }

    $type = '';

    if(!array_key_exists('type', $form_state['values']['step_elements']) && !empty($name)) {

        if(!empty($form_state['wizzard_params']['type'])) {
            $type = $form_state['wizzard_params']['type'];
        } else {
            $connector_definition = so_feedsagent_get_connectors_definitions($form_state['wizzard_params']['connector']);
            $connector = $connector = so_feedsagent_load_connector(null, $connector_definition);

            $type = $connector_definition->id . '_' . $connector->sanitizeFieldName(
                $connector_definition->id . '_',
                $name
            );
        }
    } else {
        $type = $form_state['values']['step_elements']['type'];
    }

    $description = "";

    if(array_key_exists('content_type_description', $form_state['values']['step_elements'])) {
        $description = $form_state['values']['step_elements']['content_type_description'];
    } elseif(array_key_exists('content_type_description', $form_state['wizzard_params'])) {
        $description = $form_state['wizzard_params']['content_type_description'];
    }

    $step_elements = array(
        'content_type_name' => array(
            '#type' => 'textfield',
            '#title' => t("Human readable name"),
            '#default_value' => $name,
            '#required' => true,
        ),

        'type' => array(
            '#type' => 'textfield',
            '#title' => t("Machine name"),
            '#default_value' => $type,
            '#required' => true,
        ),

        'content_type_description' => array(
            '#type' => 'textarea',
            '#title' => t("Description"),
            '#default_value' => $description,
        ),
    );
}

function so_feedsagent_generator_content_type_params_step_submission(&$form_state, $obsolescence) {

    $existing = node_type_get_names();

    if(array_key_exists($form_state['values']['step_elements']['type'], $existing)) {

        drupal_set_message(
            t("The node type '@type' ('@label') already exists.", array(
                '@type' => $form_state['values']['step_elements']['type'],
                '@label' => $existing[$form_state['values']['step_elements']['type']],
            )),
            'error'
        );

        return false;
    }
}

function so_feedsagent_generator_type_registration_step(&$step_elements, $form_state, $args = array()) {

    $wizzard_params = $form_state['wizzard_params'];

    $connector_definition = so_feedsagent_get_connectors_definitions($wizzard_params['connector']);

    $report = array(
        'node_type_created' => '',
        'node_type_edited' => '',
        'fields_definitions' => 0,
        'groups_definitions' => 0,
        'fields_instantiations' => 0,
        'feed_fields_registrations' => 0,
        'nodes_deletions' => 0,
    );

    //----- Création du node type

    if(!array_key_exists($wizzard_params['type'], node_type_get_names())) {

        $node_type = array(
            'type' => $wizzard_params['type'],
            'name' => $wizzard_params['content_type_name'],
            'base' => 'node_content',
            'description' => $wizzard_params['content_type_description'],
            'custom' => true,
            'locked' => true,
            'module' => 'so_feedsagent'
        );

        $node_type = node_type_set_defaults($node_type);
        node_type_save($node_type);

        variable_set('additional_settings__active_tab_' . $node_type->type, 'edit-menu');

        variable_set('node_preview_' . $node_type->type, 1); // facultatif
        variable_set('node_options_' . $node_type->type, array(0 => 'status'));
        variable_set('node_submitted_' . $node_type->type, 0); // user et date de soumission : off

        variable_set('comment_' . $node_type->type, 0); // pas de commentaires

        variable_set('menu_parent_' . $node_type->type, 'main-menu:0');
        variable_set('menu_options_' . $node_type->type, array(0 => 'main-menu'));

        variable_set('language_content_type_' . $node_type->type, 2); // multilingue : activé avec traductions

        node_type_cache_reset();

        $report['node_type_created'] = $node_type->type;

    } else {
        $node_type = node_type_load($wizzard_params['type']);
        $report['node_type_edited'] = $node_type->type;
    }

    //----- Génération des champs

    $groups_children = array_fill_keys(array_keys($wizzard_params['groups']), array());

    foreach($wizzard_params['fields'] as $feed_field => $infos) {

        if($infos['type'] == 'custom') {continue;}

        $field_definition = field_info_field($infos['field']);

        if(!empty($infos['group'])) {
            $groups_children[$infos['group']][] = $infos['field'];
        }

        $connector = so_feedsagent_load_connector(null, $connector_definition);
        $connector_features = $connector::getFeatures();

        $dynamic_allowed_values = array_key_exists('dynamic_allowed_values_only', $connector_features)
                                        && $connector_features['dynamic_allowed_values_only'] == true;


        // définition d'un nouveau champ
        if($field_definition == null) {

            $definition_shape = 'so_feedsagent_' . $infos['type'] . '_definition';
            module_load_include('inc', 'so_feedsagent', 'fields_types/' . $definition_shape);

            // les valeurs des champs ont été omises dans les entrées de [fields], afin de limiter le volume des données
            // et de ne pas stocker inutilement ces infos en BDD (cache form + so_feedsagent_content_types)
            $options = array(
                'multiple' => $infos['multiple'], // cardinalité
                'values' => $dynamic_allowed_values != true ? $wizzard_params['feed_fields'][$feed_field]['values'] : array(),
                // comme il s'agit d'une création de champ,  on définit ici l'image par défaut GLOBALE.
                // on prend donc celle du connecteur, qui est commune à tous les types.
                'default_image' => $connector_definition->params['generator']['widgets_options']['image']['default'],
                'label' => $infos['label'], // spécial checkboxes 'on/off'
            );

            $field_definition = $definition_shape($infos['field'] ,$options);
            $field_definition = field_create_field($field_definition);

            if($infos['type'] == 'image' && !empty($connector_definition->params['generator']['widgets_options']['image']['default'])) {

                // cette image existe déjà dans la BDD : elle a été uploadée dans les paramètres du connecteur
                $file = file_load($connector_definition->params['generator']['widgets_options']['image']['default']);
                file_usage_add($file, 'image', 'default_image', $field_definition['id']);
            }

            $report['fields_definitions']++;
        }

        $field_instance = field_info_instance('node', $infos['field'], $node_type->type);

        // instanciation du champ
        if($field_instance == null) {

            $instance_shape = 'so_feedsagent_' . $infos['type'] . '_instance';
            module_load_include('inc', 'so_feedsagent', 'fields_types/' . $instance_shape);

            $options = array(
                'multiple' => $infos['multiple'], // uniquement utilisé pour le placement des labels

                // cette fois, l'image par défaut est LOCALE
                'default_image' => $wizzard_params['params']['common']['widgets_options']['image']['default'],
                'image_folder' => $wizzard_params['params']['common']['widgets_options']['image']['folder'],
                'file_folder' => $wizzard_params['params']['common']['widgets_options']['file']['folder'],
                'weight' => $infos['weight'],
            );

            $field_instance = $instance_shape($infos['field'], $infos['label'], $node_type->type, $options);
            // field_create_instance(), contrairement à ce que prétend la documentation, retourne l'instance créée SANS L'ID (voir ci-dessous)
            field_create_instance($field_instance);

            if($infos['type'] == 'image' && !empty($wizzard_params['params']['common']['widgets_options']['image']['default'])) {

                // on récupère l'instance fraichement créée, avec son ID cette fois (voir ci-dessus)
                $field_instance = field_read_instances(array(
                    'field_id' => $field_definition['id'],
                    'bundle' => $node_type->type,
                ));

                // cette image existe déjà dans la BDD : elle vient d'être uploadée dans les paramètres des widgets
                $file = file_load($wizzard_params['params']['common']['widgets_options']['image']['default']);

                $file->status = FILE_STATUS_PERMANENT;
                file_save($file);

                file_usage_add($file, 'image', 'default_image', $field_instance[0]['id']);
            }

            $report['fields_instantiations']++;
        }
    }

    //----- Génération des groupes de champs

    $default_field_group = variable_get('default_field_group');

    foreach($wizzard_params['groups'] as $feed_group => $infos) {

        if(field_group_exists($infos['group'], 'node', $node_type->type, 'form') == false) {

            module_load_include('inc', 'so_feedsagent', 'fields_types/so_feedsagent_fieldgroup');

            $group_definition = so_feedsagent_fieldgroup(
                $infos['group'],
                $infos['label'],
                $node_type->type,
                $infos['weight'],
                $groups_children[$feed_group]
            );

            field_group_group_save($group_definition);

            $default_field_group[$group_definition->identifier] = false;

            $report['groups_definitions']++;
        }
    }

    variable_set('default_field_group', $default_field_group);

    //----- contrôle/création des dossiers des widgets

    $folder = 'public://' . $wizzard_params['params']['common']['widgets_options']['image']['folder']; // variable obligatoirement : @see https://api.drupal.org/comment/34023#comment-34023
    file_prepare_directory($folder, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);

    $folder = 'public://' . $wizzard_params['params']['common']['widgets_options']['file']['folder'];
    file_prepare_directory($folder, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);

    //----- sauvegarde du virtual type

    // Dans le cas d'une opération d'édition, si le type virtuel a été remappé sur un autre type Drupal,
    // il faut supprimer les anciens nodes, dont le type sera incohérent par rapport à la (nouvelle) réalité,
    // ce qui empêchera l'importer de faire les mises à jour.
    if(!empty($wizzard_params['id'])) {
        $type_definition = so_feedsagent_get_content_types_definitions($wizzard_params['id']);

        if($type_definition->type != $node_type->type) {

            drupal_save_session(false);
            global $user;
            $user_backup = $user;
            $user = user_load(1);

            $results = db_query("SELECT nid FROM {feedsagent_contents} WHERE type_id=?", array($wizzard_params['id']));

            while($row = $results->fetchObject()) {
                node_delete($row->nid);
                $report['nodes_deletions']++;
            }

            $user = $user_backup;
            drupal_save_session(true);
        }
    }

    $stored_content_type = array(
        'id' => $wizzard_params['id'],
        'type' => $node_type->type,
        'connector' => $wizzard_params['connector'],
        'virtual_name' => $wizzard_params['virtual_name'],
        'ignored' => 0,
        'primary_type' => $wizzard_params['primary_type'],
        'fields' => serialize($wizzard_params['fields']),
        'groups' => serialize($wizzard_params['groups']),
        'params' => serialize($wizzard_params['params']),
    );

    $db_key = !empty($wizzard_params['id']) ? 'id' : array();

    drupal_write_record('feedsagent_content_types', $stored_content_type, $db_key);

    $report['feed_fields_registrations'] = count($wizzard_params['fields']);

    $report = array_filter($report);

    $report_display = array(
        'node_type_created' => t("Node type '@type' has been created", array('@type' => $node_type->type)),
        'node_type_edited' => t("Node type '@type' has been edited", array('@type' => $node_type->type)),
        'fields_definitions' => format_plural($report['fields_definitions'], "1 new field has been defined", "@count new fields have been defined"),
        'groups_definitions' => format_plural($report['groups_definitions'], "1 new group has been defined", "@count new groups have been defined"),
        'fields_instantiations' => format_plural($report['fields_instantiations'], "1 field has been instantiated", "@count fields have been instantiated"),
        'feed_fields_registrations' => format_plural($report['feed_fields_registrations'], "1 feed field has been registered", "@count feed fields have been registered"),
        'nodes_deletions' => t("Because of node type remapping") . ", " . format_plural($report['nodes_deletions'], "1 node has been deleted", "@count nodes have been deleted"),
    );

    $report_display = array_intersect_key($report_display, $report);

    $step_elements = array(
        '#theme' => 'item_list',
        '#title' => t("Report"),
        '#type' => 'ul',
        '#items' => $report_display,
    );

    field_cache_clear();
}

/**
 * Correlate a given mixed field types list with database stored ones.
 * Index the results in five sub-arrays :
 *              - 'virtual_type' : fields mappings in present virtual type
 *              - 'drupal_type' : fields mappings in present Drupal's type
 *              - 'connector' : fields mappings in present connector
 *              - 'compatibilities' : possibly compatible fields :
 *                          - 'virtual_type' : in present virtual type
 *                          - 'drupal_type' : in present Drupal's type
 *                          - 'connector' : in present connector
 *              - 'drupal_fields' : fields' infos indexed by Drupal's fields, used to retrieve mappings infos
 *
 * @param string $connector_id
 * @param array $fields : associative array of feed fields indexed by field name
 * @param string $primary_type
 * @param string $drupal_type
 *
 * @return array
 */
function so_feedsagent_generator_gather_existing_fields_mappings($connector_id, $fields, $primary_type = '', $drupal_type = '') {

    // on obtient notamment une liste de types vers lesquels un type donné peut être mappé
    $available_fields_types = so_feedsagent_get_available_fields_types();

    // on liste quels types peuvent être mappés sur un type donné
    $retro_compatibilities = array_fill_keys(array_keys($available_fields_types), array());

    foreach($available_fields_types as $field_type => $infos) {
        foreach($infos['compatible_types'] as $compatible_type) {
            $retro_compatibilities[$compatible_type][] = $field_type;
        }
    }

    $retro_compatibilities = array_filter($retro_compatibilities);

    $mappings_default_fill = array_fill_keys(array_keys($available_fields_types), array());

    $mappings = array(
        'virtual_type' => array(),
        'drupal_type' => array(),
        'connector' => array(),
        'compatibilities' => array(
            'virtual_type' => $mappings_default_fill,
            'drupal_type' => $mappings_default_fill,
            'connector' => $mappings_default_fill,
        ),
    );

    // on rassemble l'ensemble des champs stockés appartenant à un connecteur donné
    $results = db_query("SELECT connector, type, primary_type, fields, groups FROM {feedsagent_content_types} WHERE connector=? AND ignored=0", array($connector_id));

    // on passe en revue l'ensemble des champs déjà mappés du connecteur auquel appartient le type actuellement édité
    while($row = $results->fetchObject()) {
        $stored_fields = unserialize($row->fields);

        foreach($stored_fields as $stored_field => $field_infos) {

            $mapping_option = $field_infos['field'] . " (" . $available_fields_types[$field_infos['type']]['label'] . ")";
            $mappings['drupal_fields'][$field_infos['field']] = $field_infos;

            // le champ a été sélectionné dans le type virtuel en cours d'édition
            if(array_key_exists($stored_field, $fields)) {

                if($row->primary_type == $primary_type) {
                    $mappings['virtual_type'][$stored_field][$field_infos['field']] = $mapping_option;
                } elseif($row->type == $drupal_type) {
                    $mappings['drupal_type'][$stored_field][$field_infos['field']] = $mapping_option;
                }

                $mappings['connector'][$stored_field][$field_infos['field']] = $mapping_option;
            }

            // on inscrit le champs dans chaque container de types de champ avec lesquels il est réputé compatible

            $mapping_option = array($field_infos['field'] => $field_infos['field'] . " (" . $available_fields_types[$field_infos['type']]['label'] . ")");

            if($row->primary_type == $primary_type) {
                // il est bien-entendu compatible avec son propre type
                $mappings['compatibilities']['virtual_type'][$field_infos['type']] += $mapping_option;

                foreach($retro_compatibilities[$field_infos['type']] as $compatibility) {
                    $mappings['compatibilities']['virtual_type'][$compatibility] += $mapping_option;
                }
            } elseif($row->type == $drupal_type) {
                $mappings['compatibilities']['drupal_type'][$field_infos['type']] += $mapping_option;

                foreach($retro_compatibilities[$field_infos['type']] as $compatibility) {
                    $mappings['compatibilities']['drupal_type'][$compatibility] += $mapping_option;
                }
            }

            $mappings['compatibilities']['connector'][$field_infos['type']] += $mapping_option;

            foreach($retro_compatibilities[$field_infos['type']] as $compatibility) {
                $mappings['compatibilities']['connector'][$compatibility] += $mapping_option;
            }
        }
    }

    $mappings['drupal_type'] = array_diff_key($mappings['drupal_type'], $mappings['virtual_type']);
    $mappings['connector'] = array_diff_key($mappings['connector'], $mappings['virtual_type'], $mappings['drupal_type']);

    foreach($mappings_default_fill as $type => $infos) {
        $mappings['compatibilities']['drupal_type'][$type] = array_diff_key($mappings['compatibilities']['drupal_type'][$type], $mappings['compatibilities']['virtual_type'][$type]);
        $mappings['compatibilities']['connector'][$type] = array_diff_key($mappings['compatibilities']['connector'][$type], $mappings['compatibilities']['virtual_type'][$type], $mappings['compatibilities']['drupal_type'][$type]);
    }

    return $mappings;
}

/**
 * Correlate a given mixed field/group list with database stored ones.
 * Index the results in three sub-arrays :
 *              - 'virtual_type' : fields/groups in present virtual type
 *              - 'drupal_type' : fields/groups in present Drupal's type
 *              - 'connector' : fields/groups already used with the present connector
 *
 * @param array $form_state
 *
 * @return array
 */
function so_feedsagent_generator_gather_existing_fields($form_state) {

    $connector_id = $form_state['wizzard_params']['connector'];
    $fields = $form_state['wizzard_params']['feed_fields']; // seules les clés sont utiles
    $drupal_type = $form_state['wizzard_params']['type'];

    $existing_fields = array(
        'virtual_type' => array(),
        'drupal_type' => array(),
        'connector' => array(),
    );

    $results = db_query("SELECT connector, type, primary_type, fields, groups FROM {feedsagent_content_types} WHERE connector=? AND ignored=0", array($connector_id));

    while($row = $results->fetchObject()) {
        $stored_fields = unserialize($row->fields);
        $stored_groups = unserialize($row->groups);

        foreach($stored_fields as $stored_field => $field_infos) {

            if(array_key_exists($stored_field, $fields)) {

                if($row->type == $drupal_type) {
                    $existing_fields['drupal_type'][$stored_field] = 1;
                } else {
                    $existing_fields['connector'][$stored_field] = 1;
                }
            }
        }

        foreach($stored_groups as $stored_group => $group_infos) {

            if(array_key_exists($stored_group, $fields)) {

                if($row->type == $drupal_type) {
                    $existing_fields['drupal_type'][$stored_group] = 1;
                } else {
                    $existing_fields['connector'][$stored_group] = 1;
                }
            }
        }
    }

    $existing_fields['virtual_type'] = array_fill_keys(array_keys($form_state['wizzard_params']['fields']), 1);
    $existing_fields['virtual_type'] += array_fill_keys(array_keys($form_state['wizzard_params']['groups']), 1);

    $existing_fields['drupal_type'] = array_diff_key($existing_fields['drupal_type'], $existing_fields['virtual_type']);
    $existing_fields['connector'] = array_diff_key($existing_fields['connector'], $existing_fields['virtual_type'], $existing_fields['drupal_type']);

    $existing_fields = array_filter($existing_fields);

    return $existing_fields;
}

function so_feedsagent_generator_feed_fields_step(&$step_elements, &$form_state, $args = array()) {

    _so_feedsagent_register_dependency($form_state, 'existing_feed_fields', array('feed_fields', 'type'));
    _so_feedsagent_register_dependency($form_state, '_feed_fields_selected', array('feed_fields'));

    // si les conditions ont changé, on réinitialise les valeurs restorées à partir du storage
    if(_so_feedsagent_get_dependency_state($form_state, '_feed_fields_selected') == true) {
        $form_state['values']['step_elements'] = array();
    }

    $available_fields_types = so_feedsagent_get_available_fields_types();

    $fields = array();
    $lockeds = array();
    $existing_fields_classes = array();
    $selecteds_options = array();
    $labels = array();
    $type_families = array();
    $structure = array();
    $fields_sorting_index = 0;

    $values_tootltip_pattern = array(
        'path' => base_path() . 'misc/message-16-info.png',
        'title' => "",
        'attributes' => array('style' => "vertical-align: middle;"),
    );

    if(!array_key_exists('existing_feed_fields', $form_state['wizzard_params'])
            || _so_feedsagent_get_dependency_state($form_state, 'existing_feed_fields') == true) {

        $form_state['wizzard_params']['existing_feed_fields'] = so_feedsagent_generator_gather_existing_fields($form_state);

        _so_feedsagent_invalidate_dependencies($form_state, array('existing_feed_fields'));
        _so_feedsagent_reset_dependency_state($form_state, 'existing_feed_fields');
    }

    // les champs/groupes sélectionnés dans le wizzard en cours
    $existing_fields_classes += array_fill_keys(array_keys($form_state['wizzard_params']['fields']), 4);
    $existing_fields_classes += array_fill_keys(array_keys($form_state['wizzard_params']['groups']), 4);

    $existing_fields_classes += array_key_exists('virtual_type', $form_state['wizzard_params']['existing_feed_fields']) ?
                                array_fill_keys(array_keys($form_state['wizzard_params']['existing_feed_fields']['virtual_type']), 4) :
                                array();

    $existing_fields_classes += !empty($form_state['wizzard_params']['custom_selection']) ?
                                array_fill_keys(array_keys($form_state['wizzard_params']['custom_selection']), 3) :
                                array();

    $existing_fields_classes += array_key_exists('drupal_type', $form_state['wizzard_params']['existing_feed_fields']) ?
                                array_fill_keys(array_keys($form_state['wizzard_params']['existing_feed_fields']['drupal_type']), 2) :
                                array();
    $existing_fields_classes += array_key_exists('connector', $form_state['wizzard_params']['existing_feed_fields']) ?
                                array_fill_keys(array_keys($form_state['wizzard_params']['existing_feed_fields']['connector']), 1) :
                                array();

    // les valeurs sont des indexes numériques pour le tableau des classes du thème
    $groups_container_classes = array();

    foreach($form_state['wizzard_params']['feed_fields'] as $field_name => $definition) {

        $is_locked = array_key_exists('locked', $definition) && $definition['locked'] == true;
        $is_sortable = !array_key_exists('prevent_sorting', $definition) || $definition['prevent_sorting'] != true;

        if($is_locked == true) {
            $structure[$field_name] = '..' . $fields_sorting_index++;
        } elseif($is_sortable == false) {
            $structure[$field_name] = '.' . $fields_sorting_index++;
        } else {
            $structure[$field_name] = $definition['label'];
        }

        if(array_key_exists('type', $definition) && $definition['type'] == 'group') { //-- groupe de champs

            $fields[$field_name] = array();

            // par défaut, on crée une entrée vide dans le tableau des classes de groupes (le tableau sera filtré ultérieurement avant d'être mergé)
            $groups_container_classes[$field_name] = null;

        } elseif(array_key_exists('group', $definition) && !empty($definition['group'])) { //-- champ de groupe
            $fields[$definition['group']][] = $field_name;

            unset($structure[$field_name]); // ce type de champ ne fait pas partie de la structure.

            // si le champ a déjà été utilisé mais pas le groupe auquel il appartient, on crée ou on supplante la classe de ce groupe,
            // afin d'indiquer la présence du champ au sein de ce groupe lorsqu'il est collapsed
            if(array_key_exists($field_name, $existing_fields_classes) && !array_key_exists($definition['group'], $existing_fields_classes)) {
                $groups_container_classes[$definition['group']]  = max($groups_container_classes[$definition['group']], $existing_fields_classes[$field_name]);
            }

        } elseif(array_key_exists('type', $definition) && !empty($definition['type'])) { //-- champ simple

            $fields[$field_name] = $field_name;
        }

        if($is_locked == true) {
            $lockeds[$field_name] = true;
        } else {
            $selecteds_options[$field_name] = '';
        }

        $labels[$field_name] = array_key_exists('label', $definition) && !empty($definition['label']) ? array('#markup' => $definition['label']) : array('#markup' => $field_name);
        $type_families[$field_name] = array_key_exists('type', $definition) ? array('#markup' => $available_fields_types[$definition['type']]['family']) : array('#markup' => t("undefined"));

        if(array_key_exists('values', $definition) && count(array_filter($definition['values'], strlen)) > 0) {
            $values_tooltip = $values_tootltip_pattern;
            $values_tooltip['title'] = implode(PHP_EOL, $definition['values']);
            $type_families[$field_name]['#suffix'] = " " . theme('image', $values_tooltip);
        }
    }

    natcasesort($structure);

    foreach($structure as $field_name => $sorting) {

        $structure[$field_name] = $fields[$field_name];
    }

    $classes = array(
        4 => 'in_virtual_type',
        3 => 'in_custom_selection',
        2 => 'in_drupal_type',
        1 => 'in_connector',
    );

    foreach($existing_fields_classes as $index => $val) {
        $existing_fields_classes[$index] = $classes[$val];
    }

    $groups_container_classes = array_filter($groups_container_classes);

    foreach($groups_container_classes as $index => $val) {
        $existing_fields_classes[$index] = $classes[$val] . '_container';
    }

    $features_classes = array();

    $features_classes[] = 'subfields_autocheck'; // autocheck des sous-champs de groupe

    $legend_select_pattern = array(
        '#type' => 'checkbox',
        '#title' => null,
        '#attributes' => array('class' => array()),
    );

    $in_virtual_type_select_all = $legend_select_pattern;
    $in_virtual_type_select_all['#attributes']['class'][] = 'in_virtual_type_select_all';
    $in_virtual_type_select_all['#attributes']['checked'] = 'checked';

    $in_custom_selection_select_all = $legend_select_pattern;
    $in_custom_selection_select_all['#attributes']['class'][] = 'in_custom_selection_select_all';

    $in_drupal_type_select_all = $legend_select_pattern;
    $in_drupal_type_select_all['#attributes']['class'][] = 'in_drupal_type_select_all';

    $in_connector_select_all = $legend_select_pattern;
    $in_connector_select_all['#attributes']['class'][] = 'in_connector_select_all';

    $selected_default = array();
    $selected_default += array_key_exists('selecteds', $form_state['values']['step_elements']) ? array_keys(array_filter($form_state['values']['step_elements']['selecteds'])) : array();
    $selected_default += array_merge($selected_default, array_keys($form_state['wizzard_params']['fields']));
    $selected_default += array_merge($selected_default, array_keys($form_state['wizzard_params']['groups']));

    $step_elements = array(
        '#type' => 'container',
        '#theme' => 'so_feedsagent_generator_feed_fields_overview',
        '#attributes' => array(
            'class' => $features_classes,
        ),

        '#structure' => $structure, // organisation des champs en champs simples, groupes de champs et champs de groupe.
        '#existing_fields_classes' => $existing_fields_classes,

        'lockeds' => array(
            '#type' => 'value',
            '#value' => $lockeds,
        ),

        'legend' => array(
            '#type' => 'fieldset',
            '#title' => t("Legend"),
            '#attributes' => array('class' => array('fields_overview_legend')),

            'content' => array(
                '#markup' => "<div class='legend_item'><div class='symbol in_virtual_type_legend'></div><span>" . t("Used in the virtual type") . "</span>" .
                             render($in_virtual_type_select_all) . "</div>" .
                             "<div class='legend_item'><div class='symbol in_drupal_type_legend'></div><span>" . t("Used in the Drupal's type") . "</span>" .
                             render($in_drupal_type_select_all) . "</div>" .
                             "<div class='legend_item'><div class='symbol in_connector_legend'></div><span>" . t("Used in the connector") . "</span>" .
                             render($in_connector_select_all) . "</div>",
            ),
        ),

        'selecteds' => array(
            '#type' => 'checkboxes',
            '#title' => null,
            '#options' => $selecteds_options,
            '#default_value' => $selected_default,
        ),

        'labels' => $labels,

        'type_families' => $type_families,
    );

    if(!empty($form_state['wizzard_params']['custom_selection'])) {

        $step_elements['legend']['content']['#markup'] .= "<div class='legend_item'><div class='symbol in_custom_selection_legend'></div><span>" . $form_state['wizzard_params']['custom_selection_label'] . "</span>" .
                                                          render($in_custom_selection_select_all) . "</div>";
    }

    _so_feedsagent_reset_dependency_state($form_state, '_feed_fields_selected');

    return array(
        '#attributes' => array(
            'class' => array(
                // avant soumission, les champs vides sont retirés du DOM afin qu'ils ne transitent par $_POST :
                'form_autoclean',
            ),
        ),
    );
}

function so_feedsagent_generator_feed_fields_step_submission(&$form_state, $obsolescence) {

    if($obsolescence == true) {

        _so_feedsagent_register_dependency($form_state, 'fields');

        $form_state['values']['step_elements']['selecteds'] = array_filter($form_state['values']['step_elements']['selecteds']);

        $selected_fields = array_intersect_key($form_state['wizzard_params']['feed_fields'], $form_state['values']['step_elements']['selecteds']);
        $locked_fields = array_intersect_key($form_state['wizzard_params']['feed_fields'], $form_state['values']['step_elements']['lockeds']);

        $fields = $locked_fields + $selected_fields;

        // on supprime les champs et groupes non retenus...
        $form_state['wizzard_params']['fields'] = array_intersect_key($form_state['wizzard_params']['fields'], $fields);
        $form_state['wizzard_params']['groups'] = array_intersect_key($form_state['wizzard_params']['groups'], $fields);

        // ... mais on n'inscrit que les champs et groupes qui ne le sont pas déjà (pour ne pas écraser les infos, lors d'une édition de type virtuel)
        $fields = array_diff_key($fields, $form_state['wizzard_params']['fields'], $form_state['wizzard_params']['groups']);

        $weight = 1;

        foreach($fields as $field_name => $infos) {
            if(!array_key_exists('type', $infos)) {continue;}

            if(array_key_exists($field_name, $locked_fields)) {

                if($infos['type'] == 'group') {

                    $form_state['wizzard_params']['groups'][$field_name] = array('weight' => $weight++);

                } else {

                    $form_state['wizzard_params']['fields'][$field_name] = array(
                        'type' => (string)$infos['type'],
                        'multiple' => (bool)$infos['multiple'],
                        'locked' => true,
                        'weight' => $weight++,
                    );
                }
            } else {

                if($infos['type'] == 'group') {

                    $form_state['wizzard_params']['groups'][$field_name] = array(
                        'weight' => $weight++,
                    );

                } else {

                    $form_state['wizzard_params']['fields'][$field_name] = array(
                        'multiple' => (bool)$infos['multiple'],
                        'weight' => $weight++,
                    );
                }
            }

            if(array_key_exists('group', $infos)) {
                $form_state['wizzard_params']['fields'][$field_name]['group'] = $infos['group'];
            }
        }

        _so_feedsagent_invalidate_dependencies($form_state, array('fields'));
    }

    $form_state['values']['step_elements'] = array();
}

function theme_so_feedsagent_generator_feed_fields_overview($vars) {
    $output = "";

    $element = $vars['element'];

    $rows = array();

    $select_all = array(
        '#type' => 'checkbox',
        '#title' => t("Unionize"),
        '#attributes' => array('class' => array('select_all_fields')),
    );

    $headers = array(
        render($select_all),
        t("Field label"),
        t("System name"),
        t("Type family"),
    );

    $group_content_element_pattern = array(
        '#type' => 'container',
        '#theme' => 'so_feedsagent_generator_feed_fields_overview',
        '#attributes' => array(), // obligatoire : bug Drupal
        '#structure' => array(),
        '#existing_fields_classes' => $element['#existing_fields_classes'],
        'lockeds' => $element['lockeds'],
        'selecteds' => $element['selecteds'],
        'labels' => $element['labels'],
        'type_families'=> $element['type_families'],
    );

    $output .= render($element['legend']); // non transmis aux enfants

    foreach($element['#structure'] as $index => $entry) {

        $field = !is_array($entry) ? $entry : $index;

        if(array_key_exists($field, $element['lockeds']['#value'])) {
            $lock = array(
                'path' => base_path() . 'misc/permissions.png',
                'title' => t("Locked"),
                'attributes' => array('style' => "vertical-align: middle;"),
            );

            $select = array('#markup' => theme('image', $lock));
        } else {
            $select = $element['selecteds'][$field];
        }

        $field_name = explode('__', $field);

        if(!is_array($entry)) {

            $rows[] = array(
                'data' => array(
                    array(
                        'data' => render($select),
                        'class' => array('field_select'),
                    ),
                    render($element['labels'][$field]),
                    $field_name[0],
                    render($element['type_families'][$field]),
                ),
                'class' => array($element['#existing_fields_classes'][$field]),
            );

        } else {

            $group_content_element = $group_content_element_pattern;
            $group_content_element['#structure'] = $entry;

            $group = array(
                '#type' => 'fieldset',
                '#title' => render($element['labels'][$field]) . " (" . $field . ")",
                '#value' => render($group_content_element),

                '#attached' => array( // @see issue : https://drupal.org/node/1099132
                    'js' => array(
                        'misc/form.js',
                        'misc/collapse.js',
                    ),
                ),
                '#attributes' => array( // idem
                    'class' => array('collapsible', 'collapsed'),
                ),
            );

            $rows[] = array(
                'data' => array(
                    array(
                        'data' => render($select),
                        'class' => array('field_select', 'master_checkbox_cell'),
                    ),
                    array(
                        'data' => render($group),
                        'colspan' => count($headers) - 1,
                    ),
                ),
                'class' => array($element['#existing_fields_classes'][$field]),
            );
        }
    }

    $output .= theme('table', array('header' => $headers, 'rows' => $rows));

    return $output;
}

function so_feedsagent_generator_fields_definition_step(&$step_elements, &$form_state, $args = array()) {

    _so_feedsagent_register_dependency($form_state, 'fields_mappings', array('fields', 'type'));

    $connector_definition = so_feedsagent_get_connectors_definitions($form_state['wizzard_params']['connector']);
    $connector = so_feedsagent_load_connector(null, $connector_definition);
    $connector_features = $connector::getFeatures();

    $structure = array();
    $title_fields = array();
    $labels = array();
    $new_drupal_fields = array();
    $mappings_proposals = array();
    $new_fields_classes = array();
    $types = array();
    $multiples_options = array();
    $weights = array();

    $available_fields_types = so_feedsagent_get_available_fields_types();
    $fields_types_multiples = array();
    $compatible_fields_types = array();
    $fields_types_options = array();

    //----- existing mappings

    if(!array_key_exists('fields_mappings', $form_state['wizzard_params'])
            || _so_feedsagent_get_dependency_state($form_state, 'fields_mappings') == true) {

        $form_state['wizzard_params']['fields_mappings'] = so_feedsagent_generator_gather_existing_fields_mappings(
            $form_state['wizzard_params']['connector'],
            $form_state['wizzard_params']['fields'],
            $form_state['wizzard_params']['primary_type'],
            $form_state['wizzard_params']['type']
        );

        _so_feedsagent_invalidate_dependencies($form_state, array('fields_mappings'));
        _so_feedsagent_reset_dependency_state($form_state, 'fields_mappings');
    }

    // new fields
    $new_fields_classes = array_diff_key(
        $form_state['wizzard_params']['fields'],
        (array)$form_state['wizzard_params']['existing_feed_fields']['virtual_type']
    );

    // new groups
    $new_fields_classes += array_diff_key(
        $form_state['wizzard_params']['groups'],
        (array)$form_state['wizzard_params']['existing_feed_fields']['virtual_type']
    );

    //----- fields compatibilities for fields types

    foreach($available_fields_types as $available_field_type => $infos) {

        if(array_key_exists('use_fields_compatibility', $connector_features) && $connector_features['use_fields_compatibility'] == true) {

            if($available_field_type != 'undefined') {
                $compatible_fields_types[$available_field_type] = array(
                    $available_field_type => $infos['label'], // le champ est compatible avec lui-même...
                );
            } else {
                $compatible_fields_types[$available_field_type] = array();
            }

            foreach((array)$infos['compatible_types'] as $compatible_type) {
                $compatible_fields_types[$available_field_type][$compatible_type] = $available_fields_types[$compatible_type]['label'];
            }
        }

        $fields_types_options[$available_field_type] = $infos['label'];

        if($infos['multiple'] == true && (array_key_exists('use_multiple_fields', $connector_features) && $connector_features['use_multiple_fields'] == true)) {
            $fields_types_multiples[$available_field_type] = true;
        }
    }

    $typeof_compatibility_labels = array(
        'virtual_type' => t("virtual type"),
        'drupal_type' => t("Drupal's type"),
        'connector' => t("connector"),
    );

    //---- title fields

    $title_fields_options = array('' => t("off"));
    $title_fields_options += array_combine(range(0, 4), range(1, 5));

    $stored_title_fields = array_key_exists('title_fields', $form_state['wizzard_params']['params']['common']) ?
                           array_flip($form_state['wizzard_params']['params']['common']['title_fields']) : array();

    $connector_title_fields = explode(',', $connector_definition->params['importer']['title']['title_fields']);

    //----- values tooltips

    $values_tootltip_pattern = array(
        'path' => base_path() . 'misc/message-16-info.png',
        'title' => "",
        'attributes' => array('style' => "vertical-align: middle;"),
    );

    $stored_multiples_default = array();

    $discarded_fields = array_diff_key($form_state['wizzard_params']['feed_fields'], $form_state['wizzard_params']['groups'], $form_state['wizzard_params']['fields']);
    $selected_fields = array_diff_key($form_state['wizzard_params']['feed_fields'], $discarded_fields);

    $sort_weights = array();
    $indexes = array();
    foreach($selected_fields as $field_name => $infos) {

        $target = $infos['type'] == 'group' ? 'groups' : 'fields';
        $sort_weights[$field_name] = $form_state['wizzard_params'][$target][$field_name]['weight'];

        $indexes[$field_name] = $field_name;
    }
    array_multisort($sort_weights, SORT_ASC, $indexes, $selected_fields);

    $selected_fields = array_combine($indexes, $selected_fields);

    //----- Main loop

    $weight = 1;

    foreach($selected_fields as $field_name => $definition) {

        $locked = array_key_exists('locked', $definition) && $definition['locked'] == true;
        $is_group = array_key_exists('type', $definition) && $definition['type'] == 'group';
        $field_target_array = $is_group == true ? 'groups' : 'fields'; // wizzard_params[fields/groups]

        // on supprime l'association champ/groupe si le groupe n'a pas été sélectionné.
        if(array_key_exists('group', $definition) && !array_key_exists($definition['group'], $form_state['wizzard_params']['groups'])) {
            unset($definition['group']);
        }

        if($is_group == true) { //-- groupe de champs

            // à cause de la nouvelle mécanique des weights Drupal, un groupe peut se retrouver après ses champs.
            // l'entrée existera donc déjà dans la structure, et contiendra déjà des champs. Et donc, il ne faut
            // surtout pas réinitialiser.
            if(!array_key_exists($field_name, $structure)) {
                $structure[$field_name] = array();
            }

        } elseif(array_key_exists('group', $definition) && !empty($definition['group'])) { //-- champ de groupe
            $structure[$definition['group']][] = $field_name;
        } elseif(array_key_exists('type', $definition) && !empty($definition['type'])) { //-- champ simple
            $structure[$field_name] = $field_name;
        }

        $raw_field_name = explode('__', $field_name);
        $raw_field_name = $raw_field_name[0];

        //----- labels

        $default_label = !empty($form_state['values']['step_elements']) && array_key_exists($field_name, $form_state['values']['step_elements']['labels']) ?
                         $form_state['values']['step_elements']['labels'][$field_name] : "";

        if(empty($default_label)) {
            if(array_key_exists($field_name, $form_state['wizzard_params'][$field_target_array])
                    && array_key_exists('label', $form_state['wizzard_params'][$field_target_array][$field_name])) {
                $default_label = $form_state['wizzard_params'][$field_target_array][$field_name]['label'];
            }
        }

        $default_label = empty($default_label) && array_key_exists('label', $definition) ? $definition['label'] : $default_label;
        $default_label = empty($default_label) ? $field_name : $default_label;

        $feed_field_name = array_key_exists('label', $definition) && !empty($definition['label']) ? $definition['label'] . " (" . $raw_field_name . ")" : $raw_field_name;

        $labels[$field_name] = array(
            '#type' => 'textfield',
            '#title' => null,
            '#default_value' => ucfirst($default_label),
            '#description' => $feed_field_name,
            '#states' => array(
                'visible' => array(
                    ':input[name="step_elements[mappings_proposals][' . $field_name . ']"]' => array('value' => 0),
                ),
            ),
        );

        //----- title fields

        $title_default = !empty($form_state['values']['step_elements']) && array_key_exists($field_name, $form_state['values']['step_elements']['title_fields']) ?
                         $form_state['values']['step_elements']['title_fields'][$field_name] : "";

        $title_default = array_key_exists($field_name, $stored_title_fields) ? $stored_title_fields[$field_name] : array_search($field_name, $connector_title_fields);

        $title_fields[$field_name] = array(
            '#type' => 'select',
            '#title' => null,
            '#options' => $title_fields_options,
            '#default_value' => $title_default,
        );

        //----- new Drupal's fields

        $new_drupal_field_name = !empty($form_state['values']['step_elements'])
                                    && array_key_exists($field_name, $form_state['values']['step_elements']['new_drupal_fields']) ?
                                $form_state['values']['step_elements']['new_drupal_fields'][$field_name] : "";

        if(empty($new_drupal_field_name)) {

            if($is_group == true) {

                $new_drupal_field_name = $connector->sanitizeFieldName(
                    'group_' . $form_state['wizzard_params']['connector'] . '_',
                    $raw_field_name .
                    '_' .
                    $definition['label']
                );

            } else {

                $new_drupal_field_name = $connector->sanitizeFieldName(
                    'field_' .
                    $connector_definition->id .
                    '_',
                    $raw_field_name .
                    '_' .
                    $definition['label']
                );
            }
        }

        // utilisé également par les groupes (non rendu, mais valeur par défaut transmise au callback de soumission)
        $new_drupal_fields[$field_name] = array(
            '#type' => 'textfield',
            '#title' => null,
            '#field_prefix' => "(field_" . $connector_definition->id . "_)",
            '#default_value' => $new_drupal_field_name,
            '#states' => array(
                'visible' => array(
                    ':input[name="step_elements[mappings_proposals][' . $field_name . ']"]' => array('value' => 0),
                ),
            ),
            '#autocomplete_path' => 'admin/so_feedsagent/generator/drupal_field_autocomplete',
        );

        //----- mappings selection

        if($is_group == false) {

            $mappings_proposals_default = !empty($form_state['values']['step_elements'])
                                                && array_key_exists($field_name, $form_state['values']['step_elements']['mappings_proposals']) ?
                                          $form_state['values']['step_elements']['mappings_proposals'][$field_name] : '';

            $mappings_proposals_options = array();

            // le champ a déjà été mappé dans le type de contenu Drupal, dans le cadre d'un autre type virtuel
            if(array_key_exists($field_name, $form_state['wizzard_params']['fields_mappings']['drupal_type'])) {

                // on force alors ce mapping
                $mappings_proposals_options = $form_state['wizzard_params']['fields_mappings']['drupal_type'][$field_name];
                unset($new_drupal_fields[$field_name]);

            // le champ n'a jamais été mappé auparavant,
            // ou a déjà été mappé mais dans un ou plusieurs autres types de contenu Drupal
            // ou il est déjà mappé dans le type virtuel édité
            } elseif($is_group == false) {

                $existing_options_group = t("Existing mappings in connector");

                // on propose la création d'un nouveau mapping,...
                $mappings_proposals_options[0] = t("Create new field or map on existing non-SFA one");

                // ...d'utiliser l'éventuel mapping existant (dans le cas de l'édition d'un type virtuel)
                if(array_key_exists($field_name, $form_state['wizzard_params']['fields_mappings']['virtual_type'])) {

                    $options_group = t("Current mapping");
                    $mappings_proposals_options[$options_group] = $form_state['wizzard_params']['fields_mappings']['virtual_type'][$field_name];

                    $current_mapping_key = key($form_state['wizzard_params']['fields_mappings']['virtual_type'][$field_name]);

                    // strlen(), pour conserver l'éventuelle valeur '0'
                    if(strlen($mappings_proposals_default) == 0) {
                        $mappings_proposals_default = $current_mapping_key;
                    }
                }

                // ...d'utiliser un des mappings existants dans le connecteur
                if(array_key_exists($field_name, $form_state['wizzard_params']['fields_mappings']['connector'])) {

                    if(strlen($mappings_proposals_default) == 0) {
                        reset($form_state['wizzard_params']['fields_mappings']['connector'][$field_name]);
                        $mappings_proposals_default = key($form_state['wizzard_params']['fields_mappings']['connector'][$field_name]);
                    }

                    // si le champ est locked on impose le mapping existant
                    if($locked == true) {
                        unset($mappings_proposals_options[0]);
                        $mappings_proposals_options = $form_state['wizzard_params']['fields_mappings']['connector'][$field_name];
                    } else {
                        $mappings_proposals_options[$existing_options_group] = $form_state['wizzard_params']['fields_mappings']['connector'][$field_name];
                        ksort($mappings_proposals_options[$existing_options_group]);
                    }
                }

                // ...ou enfin de mapper sur un champ éventuellement compatible (pour les non locked)
                foreach($form_state['wizzard_params']['fields_mappings']['compatibilities'] as $typeof_compatibility => $infos) {
                    if(!empty($form_state['wizzard_params']['fields_mappings']['compatibilities'][$typeof_compatibility][$definition['type']])
                            && ($locked == false)) {

                        $options_group = t("Possibly compatible fields in @typeof", array('@typeof' => $typeof_compatibility_labels[$typeof_compatibility]));

                        // on ne repropose pas un champ déjà présent dans la liste
                        $mappings_proposals_options[$options_group] = array_diff_key(
                            $form_state['wizzard_params']['fields_mappings']['compatibilities'][$typeof_compatibility][$definition['type']],
                            (array)$mappings_proposals_options[$existing_options_group]
                        );

                        // champ déjà listé et dans "Current mapping"
                        unset($mappings_proposals_options[$options_group][$current_mapping_key]);

                        ksort($mappings_proposals_options[$options_group]);
                    }
                }
            }

            $mappings_proposals[$field_name] = array(
                '#type' => 'select',
                '#title' => null,
                '#options' => $mappings_proposals_options,
                '#default_value' => $mappings_proposals_default,
            );
        }

        //----- cardinality

        // aucun problème avec le select 'type', dans la mesure où celui-ci ne propose que des champs compatibles entre-eux
        // donc forcément tous multiples ou tous non-multiples
        if($is_group == false && $locked == false && array_key_exists($definition['type'], $fields_types_multiples)) {
            $multiples_options[$field_name] = '';

            if(array_key_exists('multiple', $form_state['wizzard_params']['fields'][$field_name])
                    && $form_state['wizzard_params']['fields'][$field_name]['multiple'] == true) {
                $stored_multiples_default[] = $field_name;
            }
        }

        //----- types and values tooltips

        $values_tooltip = null;

        if(array_key_exists('values', $definition) && !empty($definition['values'])) {
            $values_tooltip = $values_tootltip_pattern;
            $values_tooltip['title'] = implode(PHP_EOL, $definition['values']);
        }

        if($locked == true) {

            $types[$field_name] = array(
                '#markup' => $definition['type'],
                '#suffix' => $values_tooltip != null ? " " . theme('image', $values_tooltip) : "",
            );

        } elseif($is_group == false) {

            if(array_key_exists($definition['type'], $compatible_fields_types)) {
                $filtered_fields_types_options = $compatible_fields_types[$definition['type']];
            } else {
                $filtered_fields_types_options = $fields_types_options;
            }

            $types_default = !empty($form_state['values']['step_elements'])
                                    && array_key_exists($field_name, $form_state['values']['step_elements']['types']) ?
                             $form_state['values']['step_elements']['types'][$field_name] : '';

            $types_default = empty($types_default) && array_key_exists('type', $form_state['wizzard_params']['fields'][$field_name]) ?
                                $form_state['wizzard_params']['fields'][$field_name]['type'] : $types_default;

            $types[$field_name] = array(
                '#type' => 'select',
                '#title' => null,
                '#options' => $filtered_fields_types_options,
                '#default_value' => $types_default,
                '#field_suffix' => $values_tooltip != null ? " " . theme('image', $values_tooltip) : "",
                '#states' => array(
                    'visible' => array(
                        ':input[name="step_elements[mappings_proposals][' . $field_name . ']"]' => array('value' => 0),
                    ),
                ),
            );
        }

        //----- weights

        $weights[$field_name] = array(
            '#type' => 'weight',
            '#title' => '',
            '#default_value' => $weight++,
            '#attributes' => array('class' => array('draggable_weight')),
            '#delta' => 500,
        );
    }

    $multiples_default = !empty($form_state['values']['step_elements']) ? array_filter(array_values($form_state['values']['step_elements']['multiples'])) : $stored_multiples_default;

    $step_elements = array(
        '#type' => 'container',
        '#theme' => 'so_feedsagent_generator_fields_definition_overview',

        '#structure' => $structure, // organisation des champs en champs simples, groupes de champs et champs de groupe.

        '#new_fields_classes' => $new_fields_classes,

        '#table_id' => 'overview_root',

        'legend' => array(
            '#type' => 'fieldset',
            '#title' => t("Legend"),
            '#attributes' => array('class' => array('fields_overview_legend')),

            'content' => array(
                '#markup' => "<div class='legend_item'><div class='symbol new_field_legend'></div><span>" . t("New fields") . "</span></div>",
            ),
        ),

        'title_fields' => $title_fields,

        'weights' => $weights,

        'labels' => $labels,

        'mappings_proposals' => $mappings_proposals,
        'new_drupal_fields' => $new_drupal_fields,

        'types' => $types,

        'multiples' => array(
            '#type' => 'checkboxes',
            '#title' => null,
            '#options' => $multiples_options,
            '#default_value' => $multiples_default,
        ),

    );

    return array(
        '#attributes' => array(
            'class' => array(
                // avant soumission, les champs vides sont retirés du DOM afin qu'ils ne transitent par $_POST :
                'form_autoclean',
            ),
        ),
    );
}

function so_feedsagent_generator_drupal_field_autocomplete($name_fragment) {

    $results = db_query(
        "SELECT DISTINCT field_name FROM {field_config} WHERE field_name LIKE ? ORDER BY field_name ASC",
        array('%' . db_like($name_fragment) . '%')
    );

    $output = array();

    while($row = $results->fetchObject()) {
        $output[$row->field_name] = $row->field_name;
    }

    return drupal_json_output($output);
}

function so_feedsagent_generator_fields_definition_step_submission(&$form_state, $obsolescence, &$debug) {// HACK : &$debug

    if($obsolescence == true) {

        _so_feedsagent_register_dependency($form_state, 'fields_mappings');

        // on supprime les propositions vides (ceux où création de champ Drupal a été demandée)
        $form_state['values']['step_elements']['mappings_proposals'] = array_filter($form_state['values']['step_elements']['mappings_proposals']);

        // on ne conserve que les types des champs à créer (NOTA : les groupes ont des type non renseignés)
        $form_state['values']['step_elements']['types'] = array_intersect_key((array)$form_state['values']['step_elements']['types'], (array)$form_state['values']['step_elements']['new_drupal_fields']);

        // les champs/groupes à créer n'ont pas encore leur nom définitif
        $new_fields = array_intersect_key($form_state['values']['step_elements']['new_drupal_fields'], $form_state['wizzard_params']['fields']);
        array_walk($new_fields, function(&$val) use($form_state) {
            if(!preg_match('#^field_[a-z0-9_]+$#', $val)) {
                $val = 'field_' . $form_state['wizzard_params']['connector'] . '_' . $val;
            }
        });

        $new_groups = array_intersect_key($form_state['values']['step_elements']['new_drupal_fields'], $form_state['wizzard_params']['groups']);
        array_walk($new_groups, function(&$val) use($form_state) {
            $val = 'group_' . $form_state['wizzard_params']['connector'] . '_' . $val;
        });

        // on constitue une liste des mappings à partir des champs et groupes à créer et des champs proposés
        $mappings = $form_state['values']['step_elements']['mappings_proposals'] + $new_fields + $new_groups;

        foreach($form_state['wizzard_params']['fields'] as $field => $infos) {

            if(array_key_exists('locked', $infos) && $infos['locked'] == true) {

                $form_state['wizzard_params']['fields'][$field] = array(
                    'field' => $mappings[$field],
                    'label' => $form_state['values']['step_elements']['labels'][$field],
                    'weight' => $form_state['values']['step_elements']['weights'][$field],
                ) + $form_state['wizzard_params']['fields'][$field];

            } else {

                if(array_key_exists($field, $form_state['values']['step_elements']['mappings_proposals'])) {
                    $label = $form_state['wizzard_params']['fields_mappings']['drupal_fields'][$form_state['values']['step_elements']['mappings_proposals'][$field]]['label'];
                    $type = $form_state['wizzard_params']['fields_mappings']['drupal_fields'][$form_state['values']['step_elements']['mappings_proposals'][$field]]['type'];
                    $multiple = $form_state['wizzard_params']['fields_mappings']['drupal_fields'][$form_state['values']['step_elements']['mappings_proposals'][$field]]['multiple'];
                } else {
                    $label = $form_state['values']['step_elements']['labels'][$field];
                    $type = $form_state['values']['step_elements']['types'][$field];
                    $multiple = $form_state['values']['step_elements']['multiples'][$field];
                }

                $form_state['wizzard_params']['fields'][$field] = array_merge($form_state['wizzard_params']['fields'][$field], array(
                    'field' => $mappings[$field],
                    'type' => $type,
                    'multiple' => (boolean)$multiple,
                    'label' => $label,
                    'weight' => $form_state['values']['step_elements']['weights'][$field],
                ));
            }
        }

        foreach($form_state['wizzard_params']['groups'] as $group => $infos) {

            $form_state['wizzard_params']['groups'][$group] = array(
                'group' => $mappings[$group],
                'label' => $form_state['values']['step_elements']['labels'][$group],
                'weight' => $form_state['values']['step_elements']['weights'][$group],
            ) + $form_state['wizzard_params']['groups'][$group];
        }

        $form_state['wizzard_params']['params']['common']['title_fields'] = array_flip(array_filter($form_state['values']['step_elements']['title_fields'], 'strlen'));
        ksort($form_state['wizzard_params']['params']['common']['title_fields']);

        _so_feedsagent_invalidate_dependencies($form_state, array('fields_mappings'));

        $weights = array();
        $indexes = array();
        foreach($form_state['wizzard_params']['groups'] as $group => $infos) {
            $weights[$group] = $infos['weight'];
            $indexes[$group] = $group;
        }
        array_multisort($weights, SORT_ASC, $indexes, $form_state['wizzard_params']['groups']);

        $form_state['wizzard_params']['groups'] = array_combine($indexes, $form_state['wizzard_params']['groups']);

        $weights = array();
        $indexes = array();
        foreach($form_state['wizzard_params']['fields'] as $field => $infos) {
            $weights[$field] = $infos['weight'];
            $indexes[$field] = $field;
        }
        array_multisort($weights, SORT_ASC, $indexes, $form_state['wizzard_params']['fields']);

        $form_state['wizzard_params']['fields'] = array_combine($indexes, $form_state['wizzard_params']['fields']);
    }

    $form_state['values']['step_elements'] = array();

    $debug = $form_state;// HACK :
}

function so_feedsagent_generator_widgets_options_step(&$step_elements, &$form_state, $args = array()) {

    $connector_definition = so_feedsagent_get_connectors_definitions($form_state['wizzard_params']['connector']);
    $general_settings = variable_get('feedsagent_settings', array());

    $widgets_settings = array();
    // $widgets_settings += (array)$form_state['values']['step_elements']; // bug @dee so_feedsagent_generator_navigation_submit()
    $widgets_settings += array_key_exists('widgets_options', $form_state['wizzard_params']['params']['common']) ? $form_state['wizzard_params']['params']['common']['widgets_options'] : array();
    $widgets_settings += $connector_definition->params['generator']['widgets_options'];
    $widgets_settings += $general_settings['generator']['widgets_options'];

    $step_elements = so_feedsagent_get_widgets_configuration_elements($widgets_settings);
    $step_elements['advice'] = array('#markup' => "<p><strong>" . t("These params are inherited from connector's settings but can be overridden here.") . "</strong></p>");
}

function so_feedsagent_generator_widgets_options_step_submission(&$form_state, $obsolescence) {

    $form_state['wizzard_params']['params']['common']['widgets_options'] = $form_state['values']['step_elements'];
    $form_state['values']['step_elements'] = array();
}

function so_feedsagent_build_nodes_types_tree($node_type) {

    $results = db_query(
        "SELECT fct.id, fct.connector, fc.label AS connector_label, fct.virtual_name, fct.primary_type, fct.fields
         FROM {feedsagent_content_types} AS fct
         LEFT JOIN {feedsagent_connectors} AS fc ON fc.id=fct.connector
         WHERE fct.type=? AND fct.ignored=0",
        array($node_type)
    );

    if(empty($results)) {return false;}

    $connector_pattern = array(
        'label' => "",
        'primary_types' => array(),
    );

    $primary_type_pattern = array(
        'virtual_name' => "",
        'id' => 0,
    );

    $tree = array(
        'connectors' => array(),
        'fields' => array(),
    );

    while($row = $results->fetchObject()) {
        if(!array_key_exists($row->connector, $tree['connectors'])) {
            $tree['connectors'][$row->connector] = $connector_pattern;
            $tree['connectors'][$row->connector]['label'] = $row->connector_label;
        }

        if(!array_key_exists($row->primary_type, $tree['connectors'][$row->connector]['primary_types'])) {
            $tree['connectors'][$row->connector]['primary_types'][$row->primary_type] = $primary_type_pattern;
            $tree['connectors'][$row->connector]['primary_types'][$row->primary_type]['virtual_name'] = $row->virtual_name;
            $tree['connectors'][$row->connector]['primary_types'][$row->primary_type]['id'] = $row->id;
        }

        $fields = unserialize($row->fields);

        foreach($fields as $field) {

            if(!array_key_exists($field['field'], $tree['fields'])) {
                $tree['fields'][$field['field']] = array();
            }

            $tree['fields'][$field['field']][] = $row->connector_label . " - " . $row->virtual_name;
        }
    }

    return $tree;
}

function theme_so_feedsagent_generator_fields_definition_overview($vars) {
    $output = "";

    $element = $vars['element'];

    $rows = array();

    $headers = array(
        "", // drag handle
        t("Weight"),
        t("Title"),
        t("Label"),
        t("Drupal field mapping"),
        t("Type"),
        !empty($element['multiples']['#options']) ? t("Mult.") : array('data' => null, 'class' => array('hide_col')),
    );

    $group_content_element_pattern = array(
        '#type' => 'container',
        '#theme' => 'so_feedsagent_generator_fields_definition_overview',
        '#attributes' => array(), // obligatoire : bug Drupal
        '#structure' => array(),
        '#new_fields_classes' => $element['#new_fields_classes'],
        '#table_id' => '',
        'title_fields' => $element['title_fields'],
        'weights' => $element['weights'],
        'labels' => $element['labels'],
        'mappings_proposals'=> $element['mappings_proposals'],
        'new_drupal_fields'=> $element['new_drupal_fields'],
        'multiples'=> $element['multiples'],
        'types'=> $element['types'],
    );

    $output .= render($element['legend']); // non transmis aux enfants

    foreach($element['#structure'] as $index => $entry) {

        $field = !is_array($entry) ? $entry : $index;

        $new_field_class = array_key_exists($field, $element['#new_fields_classes']) ? array('new_field') : array();

        if(!is_array($entry)) {

            if(array_key_exists($field, $element['multiples'])) {
                $element['multiples'][$field]['#states'] = array(
                    'visible' => array(
                        ':input[name="step_elements[mappings_proposals][' . $field . ']"]' => array('value' => 0),
                    ),
                );
            }

            $label = array(
                '#type' => 'container',
                '#attributes' => array('class' => array('description')),
                '#children' => $element['labels'][$field]['#description'],
            );
            unset($element['labels'][$field]['#description']);

            $rows[] = array(
                'data' => array(
                    '',// drag handle
                    render($element['weights'][$field]),
                    render($element['title_fields'][$field]),
                    render($element['labels'][$field]) . render($label),
                    "<div>" . render($element['mappings_proposals'][$field]) . "</div><div>" . render($element['new_drupal_fields'][$field]) . "</div>",
                    render($element['types'][$field]),
                    array(
                        'data' => render($element['multiples'][$field]),
                        'class' => array(!empty($element['multiples']['#options']) ? '' : 'hide_col'),
                    ),
                ),
                'class' => array_merge($new_field_class, array('draggable')),
            );

        } else {

            $group_content_element = $group_content_element_pattern;
            $group_content_element['#structure'] = $entry;
            $group_content_element['#table_id'] = $field;

            $group = array(
                '#type' => 'fieldset',
                '#title' => null,

                'content' => array('#markup' => render($element['labels'][$field]) . render($group_content_element)),
            );

            $rows[] = array(
                'data' => array(
                    '',// drag handle
                    render($element['weights'][$field]),
                    array(
                        'data' => render($group),
                        'colspan' => count($headers) - 1,// -1 = drag handle
                    ),
                ),
                'class' => array_merge($new_field_class, array('draggable')),
            );
        }
    }

    $output .= theme('table', array(
        'header' => $headers,
        'rows' => $rows,
        'attributes' => array('id' => $element['#table_id'] . '_fields_table'),
    ));

    drupal_add_tabledrag($element['#table_id'] . '_fields_table', 'order', 'sibling', 'draggable_weight');

    return $output;
}