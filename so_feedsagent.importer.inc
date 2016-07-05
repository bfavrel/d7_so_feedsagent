<?php

class SfaImporterException extends Exception {

    public $message;

}

function so_feedsagent_importer_overview_form($form, &$form_state) {

    $form = array();
    $form['#tree'] = true;
    $form['#attached']['css'] = array(drupal_get_path('module', 'so_feedsagent') . '/css/so_feedsagent_admin.css');

    $form['feeds_overview'] = array(
        '#type' => 'container',
        '#theme' => 'so_feedsagent_feeds_overview',
        '##warnings' => array(),
        'feeds' => array(),
        'active' => array(
            '#type' => 'checkboxes',
            '#options' => array(),
            '#default_value' => array(),
        ),
        'force_update' => array(
            '#type' => 'checkboxes',
            '#options' => array(),
            '#default_value' => array(),
        ),
    );

    $feeds_definitions = so_feedsagent_get_feeds_definitions();
    $language_list = so_feedsagent_language_list();
    $periodicities = so_feedsagent_importer_get_periodicities();
    $connector_definitions = so_feedsagent_get_connectors_definitions();

    $connectors_list = array();

    foreach ($connector_definitions as $connector_definition => $infos) {

        $class = $infos->class;
        $connector_features = $class::getFeatures();

        if (in_array('importer', $connector_features['implements']) == false) {
            continue;
        }

        $connectors_list[$connector_definition] = $infos->label;
    }

    $results = db_query("SELECT item_id, data FROM {queue} WHERE name='feedsagent_importer'");

    $feeds_errors = array();

    while($row = $results->fetchObject()) {
        $data = unserialize($row->data);

        if($data['tries'] < 2) {continue;}


        $feeds_errors[$data['fid']] = t("Importer : <br />Queue : @id<br />Creation : @created<br />Last try : @retried<br />Total tries : @tries", array(
            '@id' => $row->item_id,
            '@created' => $data['created'],
            '@retried' => $data['retried'],
            '@tries' => $data['tries'],
        ));
    }

    foreach ($feeds_definitions as $definition) {

        $form['feeds_overview']['active']['#options'][$definition->id] = '';

        if ($definition->active == 1) {
            $form['feeds_overview']['active']['#default_value'][] = $definition->id;
        }

        $form['feeds_overview']['force_update']['#options'][$definition->id] = '';

        if ($definition->params['importer']['cron_mode']['force_update'] == 1) {
            $form['feeds_overview']['force_update']['#default_value'][] = $definition->id;
        }

        $form['feeds_overview']['feeds'][$definition->id] = array(
            'label' => array('#markup' => $definition->label),
            'language' => array('#markup' => $language_list[$definition->language]),
            'connector' => array('#markup' => $connectors_list[$definition->connector]),
            'periodicity' => array('#markup' => $periodicities[$definition->periodicity]),
            'operations' => array(
                'edit' => array('#markup' => l(t("configure"), 'admin/config/so_feedsagent/feeds/edit/' . $definition->id, array('query' => drupal_get_destination()))),
                'delete' => array('#markup' => l(t("delete"), 'admin/config/so_feedsagent/feeds/delete/' . $definition->id, array('query' => drupal_get_destination()))),
                'update' => array('#markup' => l(t("update"), 'admin/content/feedsagent/importer/update/' . $definition->id, array('query' => drupal_get_destination()))),
                'import' => array('#markup' => l(t("import"), 'admin/content/feedsagent/importer/import/' . $definition->id, array('query' => drupal_get_destination()))),
            ),
        );

        $connector = so_feedsagent_load_connector($definition->connector);
        $feed_infos = $connector->importerDisplayFeedInfos((array) $definition->params['connector']);

        if (!empty($feed_infos)) {
            $form['feeds_overview']['feeds'][$definition->id]['feed_infos'] = array(
                '#type' => 'fieldset',
                '#title' => t("Feed informations"),
                '#collapsible' => true,
                '#collapsed' => true,
                'infos' => array(
                    '#theme_wrappers' => array('fieldset'),
                    '#markup' => $feed_infos,
                ),
            );
        }

        if (empty($definition->params)) {
            $form['feeds_overview']['##warnings'][$definition->id] = t("Not configured yet");

        } elseif(array_key_exists($definition->id, $feeds_errors)) {
            $form['feeds_overview']['##warnings'][$definition->id] = $feeds_errors[$definition->id];
        }
    }

    if (!empty($form['feeds_overview']['feeds'])) {
        $form['save'] = array(
            '#type' => 'submit',
            '#value' => t("Save"),
        );
    }

    $form['new_feed'] = array(
        '#type' => 'container',
        '#theme' => 'so_feedsagent_new_feed',
        'label' => array(
            '#type' => 'textfield',
            '#title' => t("Name"),
            '#description' => t("Don't add language information : it will be specified everywhere it's needed."),
        ),
        'language' => array(
            '#type' => 'select',
            '#title' => t("Language"),
            '#options' => $language_list,
            '#default_value' => language_default('language'),
        ),
        'connector' => array(
            '#type' => 'select',
            '#title' => t("Connector"),
            '#options' => $connectors_list,
        ),
        'pattern' => array(),
        'add' => array(
            '#type' => 'submit',
            '#value' => t("Add"),
            '#validate' => array('so_feedsagent_importer_overview_form_add_validate'),
            '#submit' => array('so_feedsagent_importer_overview_form_add_submit'),
        ),
    );

    foreach ($feeds_definitions as $id => $definition) {
        $form['new_feed']['pattern'][$definition->connector]['#options'][$id] = $definition->label . " (" . $language_list[$definition->language] . ")";
        $form['new_feed']['pattern'][$definition->connector]['#states'] = array(
            'visible' => array(
                ':input[name="new_feed[connector]"]' => array('value' => $definition->connector),
            ),
        );
    }

    foreach ($form['new_feed']['pattern'] as &$element) {
        $element['#type'] = 'select';
        $element['#title'] = t("Based on feed");
        $element['#description'] = t("Feed to copy parameters from.");
        $element['#options'] = array(0 => "- " . t("None") . " -") + $element['#options']; // "+" : sinon réindexation (numériques)
    }

    if(!empty($form['feeds_overview']['##warnings'])) {
        drupal_set_message(format_plural(count($form['feeds_overview']['##warnings']), "One feed is currently in error state", "@count feeds are currently in error state"), 'warning');
    }

    return $form;
}

function so_feedsagent_importer_overview_form_submit($form, &$form_state) {

    $req = "UPDATE {feedsagent_feeds}
            SET active=id IN(:id)";

    db_query($req, array(
        ':id' => $form_state['values']['feeds_overview']['active'], // pas de filtrage, pour éviter l'erreur : "IN()"
    ));

    $obsolete_forced_ids = array_diff($form_state['complete form']['feeds_overview']['force_update']['#default_value'], array_filter($form_state['values']['feeds_overview']['force_update']));
    $new_forced_ids = array_diff(array_filter($form_state['values']['feeds_overview']['force_update']), $form_state['complete form']['feeds_overview']['force_update']['#default_value']);

    $feeds_to_edit = $obsolete_forced_ids + $new_forced_ids;

    foreach($feeds_to_edit as $feed_id) {
        $definition = so_feedsagent_get_feeds_definitions($feed_id);

        $definition->params['importer']['cron_mode']['force_update'] = in_array($feed_id, $new_forced_ids) ? 1 : 0;
        $definition->params = serialize($definition->params);

        drupal_write_record('feedsagent_feeds', $definition, 'id');
    }

    drupal_set_message(t("Configuration has been saved"), 'status');
}

function so_feedsagent_importer_overview_form_add_validate($form, &$form_state) {

    if (empty($form_state['values']['new_feed']['label'])) {
        form_set_error('new_feed][label', t('!name field is required.', array('!name' => t("Name"))));
    }
}

function so_feedsagent_importer_overview_form_add_submit($form, &$form_state) {

    $new_feed = array(
        'label' => $form_state['values']['new_feed']['label'],
        'language' => $form_state['values']['new_feed']['language'],
        'connector' => $form_state['values']['new_feed']['connector'],
        'active' => 0,
        'periodicity' => 'daily',
        'params' => serialize(array()),
    );

    $feed_pattern = (array) so_feedsagent_get_feeds_definitions($form_state['values']['new_feed']['pattern'][$form_state['values']['new_feed']['connector']]);

    if (!empty($feed_pattern)) {
        $new_feed['periodicity'] = $feed_pattern['periodicity'];
        $new_feed['params'] = serialize($feed_pattern['params']);
    }

    drupal_write_record('feedsagent_feeds', $new_feed);

    drupal_set_message(t("Feed created.<br />It has to be configured now :"), 'status');
    drupal_goto('admin/config/so_feedsagent/feeds/edit/' . $new_feed['id']);
}

function so_feedsagent_importer_delete_feed_confirm($form, &$form_state, $feed_id) {

    $form['feed_id'] = array('#type' => 'value', '#value' => $feed_id);

    $feed_definition = so_feedsagent_get_feeds_definitions($feed_id);
    $language_list = so_feedsagent_language_list();

    return confirm_form(
                    $form, t("Are you sure you want to delete the feed '@feed' ?", array('@feed' => $feed_definition->label . " (" . $language_list[$feed_definition->language] . ")")), null, null, t("Delete")
    );
}

function so_feedsagent_importer_delete_feed_confirm_submit($form, &$form_state) {

    $feed_definition = so_feedsagent_get_feeds_definitions($form_state['values']['feed_id']);

    // on informe les modules qu'un flux est sur le point d'être supprimé
    module_invoke_all('so_feedsagent_feed_delete', $feed_definition);

    $req = "DELETE FROM {feedsagent_feeds} WHERE id=?";
    db_query($req, array($feed_definition->id));

    so_feedsagent_importer_batch_process_feed_deletion($feed_definition);
}

function so_feedsagent_importer_edit_feed_form($form, &$form_state, $id) {

    $general_settings = variable_get('feedsagent_settings', array());
    $feed_definition = so_feedsagent_get_feeds_definitions($id);

    drupal_set_title(t("'@feed' feed configuration", array('@feed' => $feed_definition->label)));

    $form = array(
        '#tree' => true,
        '#attached' => array(
            'css' => array(drupal_get_path('module', 'so_feedsagent') . '/css/so_feedsagent_admin.css'),
        ),
        '#attributes' => array(
            'class' => 'feedsagent_admin_form',
        ),
        'configuration' => array(
            '#type' => 'vertical_tabs',
        ),
        'submit' => array(
            '#type' => 'submit',
            '#value' => t("Save"),
        ),
        'back' => array(
            '#markup' => l(t("Back"), 'admin/config/so_feedsagent/settings/feeds'),
        ),
    );

    $language_list = so_feedsagent_language_list();

    $periodicities = so_feedsagent_importer_get_periodicities();

    $form['configuration']['common'] = array(
        '#type' => "fieldset",
        '#title' => t("Basic parameters"),
        'active' => array(
            '#type' => 'checkbox',
            '#title' => t("Active"),
            '#description' => t("Inactive feeds will be discarded from updates."),
            '#default_value' => $feed_definition->active,
        ),
        'label' => array(
            '#type' => 'textfield',
            '#title' => t("Name"),
            '#default_value' => $feed_definition->label,
        ),
        'language' => array(
            '#type' => 'select',
            '#title' => t("Language"),
            '#options' => $language_list,
            '#default_value' => $feed_definition->language,
        ),
        'periodicity' => array(
            '#type' => 'select',
            '#title' => t("Update periodicity"),
            '#options' => $periodicities,
            '#default_value' => $feed_definition->periodicity,
        ),
    );

    $form['configuration']['connector'] = array(
        '#type' => "fieldset",
    );

    $connector_definition = so_feedsagent_get_connectors_definitions($feed_definition->connector);
    $connector = so_feedsagent_load_connector($feed_definition->connector, $connector_definition);

    $connector->importerFeedConfigurationForm($form['configuration']['connector'], (array)$feed_definition->params['connector']);

    $importer_settings = array_merge(
        (array)$general_settings['importer'], (array)$connector_definition->params['importer'], (array)$feed_definition->params['importer']
    );

    $form['configuration']['importer'] = array(
        '#type' => "fieldset",
        '#title' => t("Importer"),
        'cron_mode' => array(
            '#type' => 'fieldset',
            '#title' => t("Cron mode"),
            'force_update' => array(
                '#type' => 'checkbox',
                '#title' => t("During next update, feed's items must be updated, regardless their last update date."),
                '#description' => t("This parameter will be automatically reseted after successful update"),
                '#default_value' => $importer_settings['cron_mode']['force_update'],
            ),
        ),
        'title' => array(
            '#type' => 'fieldset',
            '#title' => t("Title"),
            'title_fields' => array(
                '#type' => 'textfield',
                '#title' => t("Default title field"),
                '#description' => t("As it appears in data source.<br />Several fields can be set (separated by commas). They will be processed in order, until a title is found.") . "<br />" . t("These setting can be overriden in each content type."),
                '#default_value' => $importer_settings['title']['title_fields'],
                '#size' => 30,
            ),
        ),
        'behaviors' => array(
            '#type' => 'fieldset',
            'advice' => array(
                '#markup' => "<p><strong>" . t("These params are inherited from connector's settings but can be overridden here.") . "</strong></p>",
            ),
        ),
    );

    $form['configuration']['importer']['behaviors'] = array_merge(
            $form['configuration']['importer']['behaviors'], so_feedsagent_get_behaviors_configuration_elements($importer_settings['behaviors'])
    );

    $form['configuration']['importer']['title']['ignore_title'] = $form['configuration']['importer']['behaviors']['nodes']['ignore_title'];
    $form['configuration']['importer']['title']['ignore_title']['#default_value'] = $importer_settings['title']['ignore_title'];
    unset($form['configuration']['importer']['behaviors']['nodes']['ignore_title']);

    return $form;
}

function so_feedsagent_importer_edit_feed_form_validate($form, &$form_state) {

    $feed_definition = so_feedsagent_get_feeds_definitions($form_state['build_info']['args'][0]);

    $connector = so_feedsagent_load_connector($feed_definition->connector);
    $connector->importerFeedConfigurationFormValidate($form['configuration']['connector'], $form_state['values']['configuration']['connector']);
}

function so_feedsagent_importer_edit_feed_form_submit($form, &$form_state) {

    $configuration = $form_state['values']['configuration'];

    $params = serialize(array(
        'connector' => $configuration['connector'],
        'importer' => $configuration['importer'],
    ));

    $req = "UPDATE {feedsagent_feeds}
            SET label=:label, language=:language, periodicity=:periodicity, active=:active, params=:params
            WHERE id=:id";

    db_query($req, array(
        ':label' => $configuration['common']['label'],
        ':language' => $configuration['common']['language'],
        ':periodicity' => $configuration['common']['periodicity'],
        ':active' => $configuration['common']['active'],
        ':params' => $params,
        ':id' => $form_state['build_info']['args'][0],
    ));

    drupal_set_message(t("Configuration has been saved"), 'status');
    drupal_goto('admin/config/so_feedsagent/settings/feeds');
}

function so_feedsagent_importer_update_form($form, &$form_state) {

    drupal_set_title(t("Update FeedsAgent's contents"));

    $feeds_definitions = so_feedsagent_get_feeds_definitions();

    if (empty($feeds_definitions)) {
        drupal_set_message(t("No feed was created."), 'warning');
    }

    $queues_results = db_query("SELECT DISTINCT name FROM {queue} WHERE name LIKE 'drupal_batch%'");

    $running = array();

    while($queue_name = $queues_results->fetchColumn()) {

        $parsed_queue_name = explode(':', $queue_name);

        $row = db_query("SELECT batch, timestamp FROM {batch} WHERE bid=?", array($parsed_queue_name[1]))->fetchObject();

        $batch = unserialize($row->batch);

        if(!empty($batch) && array_key_exists('feedsagent_feed_id', $batch['sets'][$batch['current_set']])) {

            $batch_time = DateTime::createFromFormat('U', $row->timestamp)->setTimezone(new DateTimeZone('Europe/Paris'))->format('d/m/Y - H:i:s');

            $running[$batch['sets'][$batch['current_set']]['feedsagent_feed_id']] = $batch['sets'][$batch['current_set']]['title'] .
                        " - " .
                        t("launched by @username (@datetime)", array(
                            '@username' => ucfirst($batch['sets'][$batch['current_set']]['feedsagent_user_name']),
                            '@datetime' => $batch_time,
                        ));
        }
    }

    if(!empty($running)) {
        $operations = "<ul><li>" . implode("</li><li>", $running) . "</li></ul>";

        drupal_set_message(
            t("One or more updates are already running") . " : <br />" .
            $operations .
            t("Updates consume a lot of server ressources. Launch several updates in the same time may put the site down.") .
            "<br />" .
            t("If you note this situation lasts an unreasonable time, that probably means a problem happens during update and the update system is frozen.") .
            " " .
            t("Feel free to contact the Technical Support"),
            'warning'
        );
    }

    $form = array(
        '#tree' => true,
        '#attached' => array(
            'css' => array(drupal_get_path('module', 'so_feedsagent') . '/css/so_feedsagent_admin.css'),
        ),
        'overview' => array(
            '#theme' => 'so_feedsagent_importer_update',
            '##warnings' => array(),
            'feeds' => array(),
            'active' => array(
                '#type' => 'checkboxes',
                '#options' => array(),
                '#default_value' => array(),
            ),
            'periodicity' => array(),
        ),
    );

    $language_list = so_feedsagent_language_list();

    $connector_definitions = so_feedsagent_get_connectors_definitions();
    $connectors_list = array();

    foreach ($connector_definitions as $connector_definition => $infos) {
        $connectors_list[$connector_definition] = $infos->label;
    }

    foreach ($feeds_definitions as $definition) {

        if (empty($definition->params)) {
            continue;
        }

        if(!empty($running)) {
            if(array_key_exists($definition->id, $running)) {
                $update = array(
                    '#markup' => t("Currently processed"),
                );
            } else {
                $update = array(
                    '#markup' => "---",
                );
            }
        } else {
            $update = array('#markup' => l(t("update"), 'admin/content/feedsagent/importer/update/' . $definition->id, array('query' => drupal_get_destination())));
        }

        $form['overview']['active']['#options'][$definition->id] = '';

        if ($definition->active == 1) {
            $form['overview']['active']['#default_value'][] = $definition->id;
        }

        $form['overview']['periodicity'][$definition->id] = array(
            '#type' => 'select',
            '#options' => so_feedsagent_importer_get_periodicities(),
            '#default_value' => $definition->periodicity,
        );

        $form['overview']['feeds'][$definition->id] = array(
            'label' => array('#markup' => $definition->label),
            'language' => array('#markup' => $language_list[$definition->language]),
            'connector' => array('#markup' => $connectors_list[$definition->connector]),
            'operations' => array(
                'update' => $update,
            ),
        );
    }

    if (!empty($form['overview']['feeds'])) {
        $form['save'] = array(
            '#type' => 'submit',
            '#value' => t("Save"),
        );
    }

    return $form;
}

function so_feedsagent_importer_update_form_submit($form, &$form_state) {

    foreach ($form_state['values']['overview']['active'] as $id => $active) {
        $req = "UPDATE {feedsagent_feeds}
                SET active=:active,
                periodicity=:periodicity
                WHERE id=:id";

        db_query($req, array(
            ':active' => (int) !empty($active),
            ':periodicity' => $form_state['values']['overview']['periodicity'][$id],
            ':id' => $id,
        ));
    }

    drupal_set_message(t("Configuration has been saved"), 'status');
}

/**
 * Define a batch to process a feed
 *
 * @param int $feed_id
 * @param string $op : either :
 *                      - 'update' : only outdated feed's elements will be processed.
 *                      - 'cron' : same as update, except processing in non-progressive, and messages are not displayed.
 *                      - 'import' : all feed's elements will be processed.
 *                      - 'cron_import' : same as cron except items are updated regardless their last update date.
 */
function so_feedsagent_importer_batch_process_feed($feed_id, $op = 'update') {

    // ! si destination est null, on entre dans une boucle infinie !
    $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/content/feedsagent';

    $op_labels = array(
        'update' => t("Update@feed", array('@feed' => null)),
        'cron' => t("Periodic update@feed", array('@feed' => null)),
        'cron_import' => t("Periodic update@feed", array('@feed' => null)),
        'import' => t("Import@feed", array('@feed' => null)),
    );

    $batch_messages = array();

    $feed_definition = so_feedsagent_get_feeds_definitions($feed_id);
    $connector_definition = so_feedsagent_get_connectors_definitions($feed_definition->connector);
    $connector = so_feedsagent_load_connector(null, $connector_definition);
    $content_types_definitions = so_feedsagent_get_content_types_definitions(null, $connector_definition->id, null, true, 'primary_type');
    $ignored_content_types = so_feedsagent_get_ignored_content_types('primary_type');

    $batch_title = t("@op of '@feed' feed (@id)", array(
        '@feed' => "[" . $feed_definition->language . "]" . $feed_definition->label,
        '@op' => $op_labels[$op],
        '@id' => $feed_definition->id,
    ));

    $items_definitions = $connector->importerGetFeedListing($feed_definition->params['connector']);

    if(empty($items_definitions) || !is_array($items_definitions)) {

        $message = empty($items_definitions) ? t("Connector didn't return any listing data") : $items_definitions;
        watchdog('Importer', $batch_title . " : " . $message, array(), WATCHDOG_ERROR);

        if ($op == 'cron' || $op == 'cron_import') {
            return;
        }

        drupal_set_message($message, 'warning');
        drupal_goto($destination);
    }

    // on crée un tableau item_id => language => feed_id => infos à partir de {feedsagent_contents} en ne prenant que les
    // entrées correspondant à celles du listing.
    $items_pool = array();

    // dans un premier temps, on va créer les indexes a partir des types de contenus Drupal réellement présents dans le feed.
    // dans un second temps, on va récupérer les attributs de type de champ SFA (paramétrés par SFA), dans les définitions de champs.
    $content_types = array();

    $discarded_virtual_content_types = array();

    foreach ($items_definitions as $id => $item) {

        if (empty($content_types_definitions[$item['primary_type']]) && !array_key_exists($item['primary_type'], $discarded_virtual_content_types)) {

            $db_key = array();

            $virtual_content_type = array(
                'type' => '',
                'connector' => $connector_definition->id,
                'virtual_name' => $item['primary_type_name'],
                'ignored' => 0,
                'primary_type' => $item['primary_type'],
                'fields' => serialize(array()),
                'groups' => serialize(array()),
                'params' => serialize(array(
                    'common' => array(), // important de l'initialiser
                    'connector' => array(
                        'item' => $item,
                        'feed' => $feed_definition,
                    ),
                )),
            );

            // s'il existe déjà, mais est ignoré, on met à jour ses infos
            if (!empty($ignored_content_types[$item['primary_type']])) {
                $db_key = 'id';
                $virtual_content_type['id'] = $ignored_content_types[$item['primary_type']]->id;
                $virtual_content_type['ignored'] = 1;
            } else {
                if ($feed_definition->params['importer']['behaviors']['content_types']['new'] != 'prepare') {
                    $virtual_content_type['ignored'] = 1;
                } else {
                    $batch_messages[] = t("The virtual content type '@type' referenced under primary type '@primary' was created", array('@type' => $item['primary_type_name'], '@primary' => $item['primary_type'])
                    );
                }
            }

            drupal_write_record('feedsagent_content_types', $virtual_content_type, $db_key);
            $discarded_virtual_content_types[$item['primary_type']] = true;

            unset($items_definitions[$id]);
            continue;

        } elseif (!array_key_exists($item['primary_type'], $discarded_virtual_content_types) && empty($content_types_definitions[$item['primary_type']]->type)) {

            // on conserve à jour les paramètres des types de contenus virtuels orphelins
            $content_types_definitions[$item['primary_type']]->fields = serialize($content_types_definitions[$item['primary_type']]->fields);
            $content_types_definitions[$item['primary_type']]->groups = serialize($content_types_definitions[$item['primary_type']]->groups);

            $content_types_definitions[$item['primary_type']]->params['connector']['item'] = $item;
            $content_types_definitions[$item['primary_type']]->params = serialize($content_types_definitions[$item['primary_type']]->params);

            drupal_write_record('feedsagent_content_types', $content_types_definitions[$item['primary_type']], 'id');

            $discarded_virtual_content_types[$item['primary_type']] = true;
            unset($items_definitions[$id]);
            continue;

        } elseif (empty($content_types_definitions[$item['primary_type']]) || array_key_exists($item['primary_type'], $discarded_virtual_content_types)) {

            unset($items_definitions[$id]);
            continue;
        }

        $items_definitions[$id]['type'] = $content_types_definitions[$item['primary_type']]->type;
        $items_definitions[$id]['type_id'] = $content_types_definitions[$item['primary_type']]->id;

        $content_types[] = $items_definitions[$id]['type'];

        unset($items_definitions[$id]['primary_type_name']); // pas d'autre utilisation - inutile de le transmettre

        $req = "SELECT fc.nid, fc.item_id, fc.feed_id, n.language, n.tnid
                FROM {feedsagent_contents} AS fc
                LEFT JOIN {node} AS n ON n.nid=fc.nid
                WHERE fc.item_id=?";

        $result = db_query($req, array($item['id']));

        while ($row = $result->fetchObject()) {
            $items_pool[$row->item_id][$row->language][$row->feed_id] = $row;
        }
    }

    $default_language = language_default('language');
    $batch_operations = array();

    if (empty($items_definitions)) {
        $batch_messages[] = t("No node was created, imported or updated.");
    }

    foreach ($items_definitions as $item) {

        $tnids = array(); // 'source' : tnid à utiliser / 'targets' : nid des nodes à paramétrer

        // on recherche l'item dans la langue du feed en cours, dans le feed_id en cours
        if (!empty($items_pool[$item['id']][$feed_definition->language][$feed_definition->id])) {
            // on le trouve : c'est une mise à jour
            $nid = $items_pool[$item['id']][$feed_definition->language][$feed_definition->id]->nid;

            //--> on met son node à jour et on l'inscrit dans le tableau des contenus
            $batch_operations[] = array('so_feedsagent_importer_batch_node_update', array($nid, $item, $op));

        // on ne le trouve pas dans la langue du feed en cours, dans le feed_id en cours
        } else {
            // on remonte d'un cran, en le cherchant seulement dans la langue du feed en cours
            if (!empty($items_pool[$item['id']][$feed_definition->language])) {
                // on le trouve : il existe dans la même langue que celle du feed en cours, mais dans au moins un autre feed.
                // on prend le premier qui vient
                $nid = reset($items_pool[$item['id']][$feed_definition->language])->nid;

                //--> on met son node à jour et on l'inscrit dans le tableau des contenus
                $batch_operations[] = array('so_feedsagent_importer_batch_node_update', array($nid, $item, $op));

            // on ne le trouve pas dans la langue du feed en cours
            } else {
                // on remonte d'un cran, en le cherchant à la racine, par item_id
                if (!empty($items_pool[$item['id']])) {
                    // on le trouve : il existe, mais dans au moins une langue différente de celle du feed en cours
                    // la langue du feed en cours est-elle celle par défaut ?
                    if ($feed_definition->language == $default_language) {

                        //--> on donne à son tnid la valeur de son nid (automatique s'il est en langue par défaut).
                        //--> on donne aux tnid des autres node la valeur de son nid.
                        $tnids['targets'] = array();

                        foreach ($items_pool[$item['id']] as $language => $feed_ids) {
                            foreach ($feed_ids as $infos) {
                                $tnids['targets'][] = $infos->nid;
                            }
                        }

                    // la langue du feed en cours n'est pas celle par défaut
                    } else {

                        // existe t-il en langue par défaut ?
                        if (!empty($items_pool[$item['id']][$default_language])) {

                            //--> on donne à son tnid la valeur du nid de l'item en langue par défaut.
                            $first_item = reset($items_pool[$item['id']][$default_language]);
                            $tnids['source'] = $first_item->nid;
                        }
                    }

                    //--> on crée son node, on importe les données, et on l'inscrit dans le tableau des contenus.
                    $batch_operations[] = array('so_feedsagent_importer_batch_node_create', array($item, $tnids, $op));

                // on le trouve pas : il n'existe dans aucune langue
                } else {
                    //--> on crée son node, on importe les données, et on l'inscrit dans le tableau des contenus
                    //--> on donne au node la même valeur à son tnid que celle de son nid (en passant un array vide en tant que $tnid)
                    $batch_operations[] = array('so_feedsagent_importer_batch_node_create', array($item, array(), $op));
                }
            }
        }
    }

    $content_types = array_unique($content_types);
    $content_types_pool = so_feedsagent_importer_get_content_types_pool($content_types, $connector_definition);

    $req = "SELECT nid, item_id FROM {feedsagent_contents} WHERE feed_id=?";
    $result = db_query($req, array($feed_definition->id));

    $feeds_previous_contents = array();

    while ($row = $result->fetchObject()) {
        $feeds_previous_contents[$row->item_id] = $row->nid;
    }

    $missing_contents = array_diff_key($feeds_previous_contents, $items_definitions);

    foreach ($missing_contents as $item_id => $nid) {
        $batch_operations[] = array('so_feedsagent_importer_batch_missing_items', array($nid, $item_id, $op));
    }

    global $user;
    $user_name = $user->uid == 0 ? "System" : $user->name;

    $batch_definition = array(
        'feedsagent_feed_id' => $feed_id,
        'feedsagent_user_name' => $user_name,
        'title' => $batch_title,
        'configuration' => array(
            'content_types_pool' => $content_types_pool,
            'feed_definition' => $feed_definition,
            'connector_definition' => $connector_definition,
            'connector' => $connector,
            'batch_messages' => $batch_messages,
        ),
        'operations' => $batch_operations,
        'finished' => 'so_feedsagent_importer_batch_finished',
        'init_message' => t("Calling feed's server an preparing data"),
        'progress_message' => t("Processing node @current out of @total"),
        'file' => drupal_get_path('module', 'so_feedsagent') . '/so_feedsagent.importer.inc',
    );

    batch_set($batch_definition);

    $batch = &batch_get();
    $batch['op'] = $op;
    $batch['title'] = $batch_title;

    if ($op == 'cron' || $op == 'cron_import') {
        // workaround : @see https://drupal.org/node/638712
        $batch['progressive'] = false;
    }

    batch_process($destination);
}

/**
 * Process nodes in case of feed deletion, according to feed's importer settings.
 *
 * @param stdclass $feed_definition
 */
function so_feedsagent_importer_batch_process_feed_deletion($feed_definition) {

    $batch_title = t("@op of '@feed' feed", array(
        '@feed' => "[" . $feed_definition->language . "]" . $feed_definition->label,
        '@op' => t("Deletion"),
            ));

    $req = "SELECT nid, item_id FROM {feedsagent_contents} WHERE feed_id=?";
    $result = db_query($req, array($feed_definition->id));

    $batch_operations = array();

    while ($row = $result->fetchObject()) {
        $batch_operations[] = array('so_feedsagent_importer_batch_missing_items', array($row->nid, $row->item_id, 'deletion'));
    }

    $batch_definition = array(
        'title' => $batch_title,
        'configuration' => array(
            'feed_definition' => $feed_definition,
            'batch_messages' => array(t("Feed '@feed' has been deleted.", array('@feed' => $feed_definition->label))),
        ),
        'operations' => $batch_operations,
        'finished' => 'so_feedsagent_importer_batch_finished',
        'init_message' => t("Calling feed's server an preparing data"),
        'progress_message' => t("Processing node @current out of @total"),
        'file' => drupal_get_path('module', 'so_feedsagent') . '/so_feedsagent.importer.inc',
    );

    batch_set($batch_definition);

    $batch = &batch_get();
    $batch['title'] = $batch_title;

    // ! si destination est null, on entre dans une boucle infinie !
    $destination = isset($_GET['destination']) ? $_GET['destination'] : 'admin/content/feedsagent';

    batch_process($destination);
}

/*
 * Gathers whole SFA fields of given Drupal's types, in a pool.
 * --> if an SFA's field is unionized in a single SFA's type only, among several SFA's type mapped on the same Drupal's type,
 *     this field will be updated for whole SFA's types.
 */
function so_feedsagent_importer_get_content_types_pool($content_types, $connector_definition) {

    $content_types_definitions = so_feedsagent_get_content_types_definitions(null, $connector_definition->id, null, false);

    $content_types_pool = array();
    $fields = array();

    foreach ($content_types as $content_type) {
        $req = "SELECT f.field_name, f.cardinality, f.translatable  FROM {field_config} AS f
                LEFT JOIN {field_config_instance} AS i ON i.field_id=f.id
                WHERE i.entity_type='node'
                AND i.bundle=?";

        $result = db_query($req, array($content_type));

        while ($row = $result->fetchObject()) {
            $fields[$row->field_name] = array(
                'cardinality' => $row->cardinality,
                'translatable' => $row->translatable,
            );
        }

        $content_types_pool[$content_type]['configuration'] = array();
        $content_types_pool[$content_type]['fields'] = array();

        foreach ($content_types_definitions as $content_types_definition) {

            if ($content_types_definition->type != $content_type) {
                continue;
            }

            $content_types_pool[$content_type]['configuration'][$content_types_definition->id] = $content_types_definition->params;

            foreach ($content_types_definition->fields as $feed_field => $field_infos) {
                $content_types_pool[$content_type]['fields'][$feed_field] = array(
                    'field' => $field_infos['field'],
                    'type' => $field_infos['type'],
                    'cardinality' => $fields[$field_infos['field']]['cardinality'],
                    'translatable' => $fields[$field_infos['field']]['translatable'],
                    'values' => array(),
                );
            }
        }
    }

    return $content_types_pool;
}

function so_feedsagent_importer_batch_node_create($item, $tnids = array(), $op, &$context) {

    $batch = batch_get();
    $configuration = &$batch['sets'][$batch['current_set']]['configuration'];

    $connector = $configuration['connector'];
    $content_types_pool = $configuration['content_types_pool'];
    $content_type = $item['type'];

    $language = $configuration['feed_definition']->language;

    // Lorsque qu'un cron n'arrive pas à terme, Drupal stocke les opérations de batch restantes en queue.
    // Au cron suivant, ces opérations ne peuvent pas être identifiées comme appartenant à un flux en particulier et sont
    // traitées en priorité.
    // Si une de ces opération est une création de node, elle va être à nouveau programmée dans la queue "officielle" de ce flux.
    // On se retrouve donc avec un node doublon de fiche.
    // Il faut donc effectuer une vérification avant toute création de node, afin de s'assurer que ce node n'a pas été créé entre-temps.

    $existing = db_query(
        "SELECT fc.nid
         FROM {feedsagent_contents} AS fc
         LEFT JOIN {node} AS n ON n.nid=fc.nid
         WHERE fc.item_id=?
         AND fc.feed_id=?
         AND n.language=?",
        array(
            $item['id'],
            $configuration['feed_definition']->id,
            $language,
        )
    )->fetchObject();

    if(!empty($existing)) {

        $configuration['batch_messages'][] = t(
            "Attempt of creation of duplicate item's node : <br />!data",
            array(
                '!data' => "Item : " . $item['id'] . "<br />Language : " . $language . "<br />Existing nid : " . $existing->nid,
            )
        );

        return;
    }

    $node = so_feedsagent_importer_node_create($content_type, $language);

    if (!empty($node)) {
        node_save($node);
        $saved_nid = $node->nid;
    }

    $item_infos = array(
        'id' => $item['id'],
        'container' => 0,
        'timestamp' => $item['date'],
    );

    $event_infos = array(
        'operation' => t("@op of feed '@feed'", array('@op' => $op, '@feed' => $configuration['feed_definition']->label)),
    );

    // title_fields du feed, supplanté par celui du virtual type
    $feed_importer_settings = $configuration['feed_definition']->params['importer'];
    $content_types_pool[$node->type]['configuration'][$item['type_id']]['common']['title_fields'] += explode(',', $feed_importer_settings['title']['title_fields']);

    try {
        set_error_handler('_so_feedsagent_importer_error_handler', E_ALL & ~E_NOTICE | E_STRICT);

        so_feedsagent_importer_node_update(
            $node,
            $item,
            $content_types_pool[$content_type]['fields'],
            $content_types_pool[$content_type]['configuration'][$item['type_id']],
            $connector,
            $configuration['feed_definition']
        );

    } catch (Exception $exc) {
        $event_infos['data'] = _so_feedsagent_parse_exception($exc);

        so_feedsagent_log_item_write('fail', $item_infos, $event_infos);

        // DEV : provisoire, en attendant la mise en place des récaps de logs
        watchdog('Importer', "Error hapend during update : <br />@data", array(
            '@data' => $event_infos['data'] . "<br />Item : " . $item['id']
        ), WATCHDOG_ERROR);
    }

    restore_error_handler();

    if (!empty($node)) { // si l'update echoue, elle vide l'objet $node

        // on traite les éventuelles erreur PDO qui sont suceptibles de survenir à la sauvegardes (ex : checkbox on/off avec des valeurs non-booléennes)
        try {
            node_save($node);
        } catch (Exception $exc) {

            global $user;
            $uid_backup = $user->uid;
            $user->uid = 1;

            node_delete($saved_nid);

            $user->uid = $uid_backup;

            $event_infos['data'] = _so_feedsagent_parse_exception($exc);

            $configuration['batch_messages'][] = t("An error occurs while saving new node matching item '@item' : <br />@data", array('@item' => $item['id'], '@data' => $event_infos['data']));
            return;
        }

        $item_infos['container'] = $node->nid;

        if (empty($exc)) {
            so_feedsagent_log_item_write('success', $item_infos, $event_infos);
        }

        $event_infos['data'] = t("Node was created and published (node type : '@type')", array('@type' => $node->type));
        so_feedsagent_log_item_write('state', $item_infos, $event_infos);

        if (!empty($tnids['source'])) {

            db_query("UPDATE {node} SET tnid=? WHERE nid=?", array($tnids['source'], $node->nid));
            $node->tnid = $tnids['source'];

        } elseif ($node->language == language_default('language')) {

            db_query("UPDATE {node} SET tnid=nid WHERE nid=?", array($node->nid));
        }

        if (!empty($tnids['targets'])) {

            db_query("UPDATE {node} SET tnid=:tnid WHERE nid IN(:nids)", array(':tnid' => $node->nid, ':nids' => $tnids['targets']));
        }

        if(!empty($item['parent_id'])) {
            $parent_nid = db_query(
                "SELECT n.nid FROM {node} AS n
                 LEFT JOIN {feedsagent_contents} AS c ON c.nid=n.nid
                 WHERE n.language=:language
                 AND c.item_id=:itemid
                 AND c.feed_id=:feedid", array(
                     ':language' => $node->language,
                     ':itemid' => $item['parent_id'],
                     ':feedid' => $configuration['feed_definition']->id,
                 ))->fetchColumn();
        } else {
            $parent_nid = $node->nid;
        }

        so_feedsagent_importer_register_content_item(
            array(
                'nid' => $node->nid,
                'item_id' => $item['id'],
                'parent_nid' => $parent_nid,
                'type_id' => $item['type_id'],
                'weight' => $item['weight'],
            ),
            $configuration['feed_definition']->id,
            time()
        );

        $context['results']['nb_created']++;
    } else {
        global $user;
        $uid_backup = $user->uid;
        $user->uid = 1;

        node_delete($saved_nid);

        $user->uid = $uid_backup;

        $configuration['batch_messages'][] = t("An error occurs while creating node matching item '@item'", array('@item' => $item['id']));
        return;
    }
}

/**
 * Create the basic structure of a node object
 *
 * @param string $content_type
 * @param string $language
 *
 * @return stdClass
 */
function so_feedsagent_importer_node_create($content_type, $language) {

    $node = new stdClass();

    $node->type = $content_type;
    $node->language = $language;

    node_object_prepare($node);
    $node = node_submit($node);

    return $node;
}

function so_feedsagent_importer_batch_node_update($nid, $item, $op, &$context) {

    $batch = batch_get();
    $configuration = $batch['sets'][$batch['current_set']]['configuration'];

    $item_infos = array(
        'id' => $item['id'],
        'container' => $nid,
        'timestamp' => $item['date'],
    );

    $event_infos = array(
        'operation' => t("@op of feed '@feed'", array('@op' => $op, '@feed' => $configuration['feed_definition']->label)),
        'start_timestamp' => time(),
    );

    // NOTA : l'item peut avoir été mis à jour plus récemment dans un autre feed
    $last_update = db_query(
        "SELECT MAX(last_update) AS last_update
        FROM feedsagent_contents AS fac
        LEFT JOIN node AS n ON n.nid=fac.nid
        WHERE item_id=? AND language=?",
        array(
            $item['id'],
            $configuration['feed_definition']->language
        )
    )->fetchColumn();

    // Si c'est un node en cours de création il sera inscrit par la fonction de création de nodes.
    // là, on ne s'occupe que de l'existant
    if($last_update !== null) {

        if(!empty($item['parent_id'])) {
            $parent_nid = db_query(
                "SELECT n.nid FROM {node} AS n
                 LEFT JOIN {feedsagent_contents} AS c ON c.nid=n.nid
                 WHERE n.language=:language
                 AND c.item_id=:itemid
                 AND c.feed_id=:feedid", array(
                     ':language' => $configuration['feed_definition']->language,
                     ':itemid' => $item['parent_id'],
                     ':feedid' => $configuration['feed_definition']->id,
                 ))->fetchColumn();
        } else {
            $parent_nid = $nid;
        }

        // deux possibilités : le node est référencé dans le feed en cours ou alors il l'est, mais dans un autre feed.
        // dans le second cas, il faut l'inscrire dans le flux en cours :
        so_feedsagent_importer_register_content_item(
            array(
                'nid' => $nid,
                'item_id' => $item['id'],
                'parent_nid' => $parent_nid,
                'type_id' => $item['type_id'],
                'weight' => $item['weight'],
            ),
            $configuration['feed_definition']->id,
            $last_update
        );
    }

    if ($op == 'update' || $op == 'cron') {

        if($last_update !== null && $item['date'] <= $last_update) {

            $item_time = DateTime::createFromFormat('U', $item['date']);
            $item_time->setTimezone(new DateTimeZone('Europe/Paris'));

            $event_infos['data'] = t("Item was up to date") . " : " . $item_time->format('d/m/Y H:i:s');
            so_feedsagent_log_item_write('info', $item_infos, $event_infos);

            $context['results']['nb_up_to_date']++;
            return;
        }

        $context['results']['nb_updated']++;
    } else {
        $context['results']['nb_imported']++;
    }

    $connector = $configuration['connector'];
    $content_types_pool = $configuration['content_types_pool'];

    $feed_importer_settings = $configuration['feed_definition']->params['importer'];

    $node = node_load($nid);
    $title_backup = $node->title;

    $nodes_types = node_type_get_names();

    so_feedsagent_log_item_write(
        'identification',
        $item_infos, array(
            'infos' => t("Title") . " : " . $node->title . " / " . t("Language") . " : " . $node->language . " / " . t("Type") . " : " . $nodes_types[$node->type],
        )
    );

    // l'item peut avoir changé de flux après sa création sur le site (fréquent avec les items des flux de type 'manifestations' du LEI)
    // Deux cas sont alors possibles : l'ancien type ne fait pas partie du pool, ou alors, ce type fait par chance, partie du pool, mais la configuration
    // correspondante au node ne sera pas trouvée dans le type attendu.
    if(!array_key_exists($node->type, $content_types_pool) || !array_key_exists($item['type_id'], $content_types_pool[$node->type]['configuration'])) {

        $new_sfa_type = db_query("SELECT virtual_name FROM {feedsagent_content_types} WHERE primary_type=?", array($item['primary_type']))->fetchColumn();

        $message = t("Node '@nid', in feed '@feed', was supposed to be part of Drupal's type '@old_drupal_type' but it's now appears to be part of SFA's type '@new_sfa_type' which is mapped on Drupal's type '@new_drupal_type'.<br />Node has been deleted.", array(
            '@nid' => $node->nid,
            '@feed' => "[" . $configuration['feed_definition']->language . "] " . $configuration['feed_definition']->label,
            '@old_drupal_type' => $node->type,
            '@new_sfa_type' => $new_sfa_type,
            '@new_drupal_type' => $item['type'],
        ));

        if ($op != 'cron' && $op != 'cron_import') {
            drupal_set_message(t("Some errors hapend during update. See logs for further details."), 'warning', false);
        }

        $event_infos['data'] = $message;
        so_feedsagent_log_item_write('fail', $item_infos, $event_infos);

        watchdog('Importer', "Node '@nid', in feed '@feed', was supposed to be part of Drupal's type '@old_drupal_type' but it's now appears to be part of SFA's type '@new_sfa_type' which is mapped on Drupal's type '@new_drupal_type'.<br />Node has been deleted.", array(
            '@nid' => $node->nid,
            '@feed' => "[" . $configuration['feed_definition']->language . "] " . $configuration['feed_definition']->label,
            '@old_drupal_type' => $node->type,
            '@new_sfa_type' => $new_sfa_type,
            '@new_drupal_type' => $item['type'],
        ), WATCHDOG_NOTICE);

        global $user;
        $uid_backup = $user->uid;
        $user->uid = 1;

        node_delete($node->nid);

        $user->uid = $uid_backup;

        return;
    }

    // title_fields du feed, supplanté par celui du virtual type
    $content_types_pool[$node->type]['configuration'][$item['type_id']]['common']['title_fields'] += explode(',', $feed_importer_settings['title']['title_fields']);

    set_error_handler('_so_feedsagent_importer_error_handler', E_ALL & ~E_NOTICE | E_STRICT);

    try {

        so_feedsagent_importer_node_update(
            $node,
            $item,
            $content_types_pool[$node->type]['fields'],
            $content_types_pool[$node->type]['configuration'][$item['type_id']],
            $connector,
            $configuration['feed_definition']
        );

    } catch (Exception $exc) {

        if ($op != 'cron' && $op != 'cron_import') {
            drupal_set_message(t("Some errors hapend during update. See logs for further details."), 'warning', false);
        }

        $event_infos['data'] = _so_feedsagent_parse_exception($exc);
        so_feedsagent_log_item_write('fail', $item_infos, $event_infos);

        // DEV : provisoire, en attendant la mise en place des récaps de logs
        watchdog('Importer', "Error hapend during update : <br />@data", array('@data' =>
            $event_infos['data'] . "<br />Item : " . $item['id'] . "<br />Nid : " . $nid,
        ), WATCHDOG_ERROR);

        restore_error_handler();
        return;
    }

    restore_error_handler();

    so_feedsagent_log_item_write('success', $item_infos, $event_infos);

    if (!empty($title_backup) && $feed_importer_settings['behaviors']['nodes']['ignore_title'] == true) {
        $node->title = $title_backup;
    }

    if($node->status == 0) {
        if($feed_importer_settings['behaviors']['nodes']['reborn'] == 'publish') {
            $node->status = 1;
            $event_infos['data'] = t("Item was existing again in feed. It has been republished.");
            $event_infos['reset'] = true;
            so_feedsagent_log_item_write('state', $item_infos, $event_infos);
        } else {
            $event_infos['data'] = t("Item was existing again in feed. It has been left unpublished.");
            $event_infos['reset'] = true;
            $event_infos['once'] = true;
            so_feedsagent_log_item_write('state', $item_infos, $event_infos);
        }
    }

    node_save($node);

    db_query("UPDATE {feedsagent_contents} SET last_update=? WHERE nid=?", array(time(), $nid));
}

/**
 * Populate a node object with values retrieved from a feed
 *
 * @param stdClass $node
 * @param array $item
 * @param array $fields : {feedsagent_content_types}.fields
 * @param array $configuration : {feedsagent_content_types}.params
 * @param object $connector
 * @param array $feed_definition
 */
function so_feedsagent_importer_node_update(&$node, $item, $fields, $configuration, $connector, $feed_definition) {

    $overrided_fields = unserialize(db_query("SELECT fields FROM {feedsagent_nodes_overrides} WHERE nid=?", array($node->nid))->fetchColumn());

    if(empty($overrided_fields)) {
        $overrided_fields = array();
    } else {
        $overrided_fields = array_flip($overrided_fields);
    }

    // on fait un premier passage pour initialiser tous les champs
    foreach ($fields as $field) {

        if (array_key_exists($field['field'], $overrided_fields)) {
            continue;
        }

        $field_language = $field['translatable'] == 1 ? $node->language : 'und';

        $node->{$field['field']}[$field_language] = array();
    }

    $title = $configuration['common']['title_fields'];

    $state = $connector->importerGetFeedValues($node, $item['id'], $title, $node->language, $configuration['connector'], $fields, $feed_definition);

    if ($state != null) {
        $exc = new SfaImporterException();
        $exc->message = $state;
        $node = null; // déclenche la suppression du node si on est en cours de création
        throw $exc;
        return;
    }

    $node->title = $title;

    $feedsagent_types = so_feedsagent_get_available_fields_types();

    foreach ($fields as $field) {

        if (array_key_exists($field['field'], $overrided_fields)) {
            continue;
        }

        // on supprime les valeurs vides, qui sinon, génèrent une entrée vide (et donc affichée en front)
        $field['values'] = array_filter($field['values'], function($val){
            return is_array($val) ? !empty($val) : strlen($val) > 0;
        });

        $format_callback = $feedsagent_types[$field['type']]['format_callback'];

        $field_language = $field['translatable'] == 1 ? $node->language : LANGUAGE_NONE;

        // on n'effectue pas le traitement si le champ n'existe pas/plus dans l'objet $node. En effet,
        // le connecteur peut, pour une raison ou une autre supprimer le champ du node, afin d'en préserver
        // les anciennes valeurs
        if(property_exists($node, $field['field'])) {
            $node->{$field['field']}[$field_language] = array_merge($node->{$field['field']}[$field_language], $format_callback(
                $field,
                $node->{$field['field']}[$node->language],
                $configuration['common']['widgets_options'],
                $node->nid,
                $item['id']
            ));
        }
    }
}

/**
 * Update/Insert a SFA content in {feedsagent_contents}
 *
 * @param array $sfa_content : contains : 'nid', 'item_id', 'parent_nid', 'type_id', 'weight'
 * @param int $feed_id
 * @param int $update_time
 */
function so_feedsagent_importer_register_content_item($sfa_content, $feed_id, $update_time = null) {

    // SEO rehabilitation
    $obsolete_since = db_query(
        "SELECT obsolete_since FROM {feedsagent_contents} WHERE nid=? AND feed_id=?",
        array(
            $sfa_content['nid'],
            $feed_id,
        )
    )->fetchColumn();

    if($obsolete_since != 0) {

        $node_infos = db_query("SELECT vid, type, language FROM {node} WHERE nid=?", array($sfa_content['nid']))->fetchObject();

        if(metatag_entity_supports_metatags('node', $node_infos->type) == true) {
            so_feedsagent_seo_indexing('activate', $sfa_content['nid'], $node_infos->vid, $node_infos->language);

            $item_infos = array(
                'id' => $sfa_content['item_id'],
                'container' => $sfa_content['nid'],
                'timestamp' => time(),
            );

            $event_infos['data'] = t("Item was existing again in feed. Scheduled unpublish/deletion has been canceled.");
            $event_infos['reset'] = true;
            so_feedsagent_log_item_write('state', $item_infos, $event_infos);
        }
    }

    db_query(
        "REPLACE INTO {feedsagent_contents}
         SET nid=:nid, nid_parent=:nidparent, feed_id=:feedid, item_id=:itemid, type_id=:typeid, weight=:weight, last_update=:lastupdate, obsolete_since=0", array(
            ':nid' => $sfa_content['nid'],
            ':nidparent' => $sfa_content['parent_nid'],
            ':feedid' => $feed_id,
            ':itemid' => $sfa_content['item_id'],
            ':typeid' => $sfa_content['type_id'],
            ':weight' => $sfa_content['weight'],
            ':lastupdate' => $update_time == null ? time() : $update_time,
        )
    );
}

function so_feedsagent_importer_single_node_update($node, $mode = false) { // TODO : à revoir : le nom de la fonction est inapproprié

    $items_query = "SELECT DISTINCT fc.item_id, fc.type_id, ff.connector FROM {feedsagent_contents} AS fc
                    LEFT JOIN {feedsagent_feeds} AS ff ON ff.id=fc.feed_id
                    WHERE fc.nid=? AND ff.id IS NOT NULL";

    $items_results = db_query($items_query, array($node->nid));

    $feeds_results = db_query(
        "SELECT ff.id,ff.label
         FROM {feedsagent_contents} AS fc
         LEFT JOIN {feedsagent_feeds} AS ff ON ff.id=fc.feed_id
         WHERE nid=?",
        array($node->nid)
    );

    if ($mode == 'launch') {

        while($items_row = $items_results->fetchObject()) {

            $feed_row = $feeds_results->fetchObject(); // c'est une mise à jour d'un fiche : on prend le premier flux qui vient qui la contient.

            $connector = so_feedsagent_load_connector($items_row->connector);
            $content_types_pool = so_feedsagent_importer_get_content_types_pool(array($node->type), $connector);
            $feed_definition = so_feedsagent_get_feeds_definitions($feed_row->id);

            $content_type_definition = so_feedsagent_get_content_types_definitions($items_row->type_id);

            if($content_type_definition->type != $node->type) {

                $message = t("An error has occured.") . "<br />" . t("Node '@nid', in feed '@feed', is supposed to be part of Drupal's type '@old_drupal_type' but it's now appears to be part of SFA's type '@new_sfa_type' which is mapped on Drupal's type '@new_drupal_type'.<br />Node has to be deleted.", array(
                    '@nid' => $node->nid,
                    '@feed' => "[" . $feed_definition->language . "] " . $feed_definition->label,
                    '@old_drupal_type' => $node->type,
                    '@new_sfa_type' => $content_type_definition->virtual_name,
                    '@new_drupal_type' => $content_type_definition->type,
                ));

                drupal_set_message($message, 'error');

                if(isset($_GET['destination'])) {
                    drupal_goto();
                } else {
                    drupal_goto('node/' . $node->nid . '/importer');
                }

                break;
            }

            $backup_title = $node->title;
            $item = array(
                'id' => $items_row->item_id,
                'type_id' => $items_row->type_id,
            );

            $item_infos = array(
                'id' => $item['id'],
                'container' => $node->nid,
            );

            $event_infos = array(
                'operation' => t("manual update of item"),
                'start_timestamp' => time(),
            );

            try {
                set_error_handler('_so_feedsagent_importer_error_handler', E_ALL & ~E_NOTICE | E_STRICT);

                so_feedsagent_importer_node_update(
                    $node,
                    $item,
                    $content_types_pool[$node->type]['fields'],
                    $content_type_definition->params,
                    $connector,
                    $feed_definition
                );

            } catch (Exception $exc) {

                $event_infos['data'] = _so_feedsagent_parse_exception($exc);

                so_feedsagent_log_item_write('fail', $item_infos, $event_infos);

                drupal_set_message(t("An error has occured. Values have not been updated from feed.<br /> See below for details."), 'error');
            }
        }

        restore_error_handler();

        if ($exc == null) {
            $node->title = $backup_title;

            node_save($node);

            db_query("UPDATE {feedsagent_contents} SET last_update=? WHERE nid=?", array(time(), $node->nid));
            so_feedsagent_log_item_write('success', $item_infos, $event_infos);
            drupal_set_message(t("Values has been updated from feed"));
        }

        if(isset($_GET['destination'])) {
            drupal_goto();
        } else {
            drupal_goto('node/' . $node->nid . '/importer');
        }
    }

    drupal_set_title(t("Update '!title' from feed", array('!title' => $node->title)));

    $build = array(
        'launch' => array(
            '#type' => 'link',
            '#title' => t("Lauch update"),
            '#href' => 'node/' . $node->nid . '/importer/launch',
            '#attributes' => array('class' => array('sfa_importer_node_lauch')),
        ),
        'report' => array(
            '#type' => 'container',
            '#attributes' => array('class' => array('sfa_importer_report_wrapper')), // bug D7 connu
        ),
    );

    $items_results = db_query($items_query, array($node->nid));

    while($items_row = $items_results->fetchObject()) {

        $connector_definition = so_feedsagent_get_connectors_definitions($items_row->connector);

        $item_url = so_feedsagent_get_item_url($items_row->item_id, $connector_definition, null, $node->nid);

        if(!empty($item_url)) {
            $item_id_display = l($items_row->item_id, $item_url, array('attributes' => array('target' => '_blank')));
        } else {
            $item_id_display = $items_row->item_id;
        }

        $build['report'][$items_row->connector] = array(
            '#type' => 'fieldset',
            '#title' => $connector_definition->label . " - " . t("Fiche ID") . " : " . $item_id_display,
            '#attributes' => array('class' => array('sfa_importer_report')),
        );

        $feeds_list_items = array();

        so_feedsagent_log_item_build($build['report'][$items_row->connector], $items_row->item_id, $node->nid);

        while($feeds_row = $feeds_results->fetchObject()) {
            $feeds_list_items[] = $feeds_row->label . " (" . t("SFA id") . " : " . $feeds_row->id . ")";
        }

        $build['report'][$items_row->connector]['feeds'] = array(
            '#type' => 'container',
            '#attributes' => array('class' => array('sfa_feeds_section')),

            'feeds' => array(
                '#theme' => 'item_list',
                '#title' => t("This node is provided by these feeds"),
                '#type' => 'ul',
                '#items' => $feeds_list_items,
            ),
        );

        // TODO : supprimer quand tous les enregistrements de la Creuse auront leur champs peuplés
        if($items_row->type_id == 0) {
            $build['launch'] = array(
                '#prefix' => "<div class='sfa_importer_node_lauch'>",
                '#markup' => "Mise à jour manuelle non disponible",
                '#suffix' => "</div>",
            );
        }

        $coupling = so_feedsagent_get_coupled_items_by_nid($node->nid);

        if(!empty($coupling)) {

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

            $build['report']['coupling'] = array(
                '#type' => 'fieldset',
                '#attributes' => array('class' => array('sfa_coupling_section')),

                'coupling' => $coupling_element,
            );
        }
    }

    return $build;
}

/**
 * Try to provides an url where a given item's data can be read.
 *
 * @param string $item_id
 * @param string/stdClass $connector_infos : a connector's definition objet, or a connector's name which must be loaded.
 * @param string $language : if no $nid and no $langue are provided, current site's language will be used.
 * @param string $nid : if provided, $language will be discarded.
 *
 * @return string/boolean : url or false
 */
function so_feedsagent_get_item_url($item_id, $connector_infos, $item_language = null, $nid = null) {

    if(is_object($connector_infos)) {
        $connector = so_feedsagent_load_connector(null, $connector_infos);
    } elseif(is_string($connector_infos)) {
        $connector = so_feedsagent_load_connector($connector_infos);
    }

    $nid_where = "";
    $nid_args = array();
    $language_where = "";
    $language_args = array();

    if($nid != null) {

        $nid_where = " AND n.nid=:nid";
        $nid_args = array(':nid' => $nid);

    } else {

        if($item_language != null) {

            $lang = $item_language;

        } else {

            global $language;
            $lang = $language->language;
        }

        $language_where = " AND (n.language=:lang1 OR n.language=:lang2)";
        $language_args = array(':lang1' => $lang, ':lang2' => 'und');
    }

    $row = db_query(
        "SELECT types.id AS virtual_type_id, feed.id AS feed_id FROM {feedsagent_contents} AS contents
        LEFT JOIN {node} AS n ON n.nid=contents.nid
        LEFT JOIN {feedsagent_content_types} AS types ON types.id=contents.type_id
        LEFT JOIN {feedsagent_feeds} AS feed ON feed.id=contents.feed_id
        WHERE contents.item_id=:itemid
        AND types.connector=:connector" . $nid_where . $language_where,
        array(':itemid' => $item_id, ':connector' => $connector->getDefinition('id')) + $nid_args + $language_args
    )->fetchObject();

    if(empty($row)) {return false;}

    return $connector->importerGetItemURL(
        $item_id,
        $lang,
        so_feedsagent_get_content_types_definitions($row->virtual_type_id),
        so_feedsagent_get_feeds_definitions($row->feed_id)
    );
}

function so_feedsagent_log_item_build(&$element, $item_id, $nid) {

    $log = so_feedsagent_log_item_read($item_id, $nid);

    $element['#attached']['css'] = array(drupal_get_path('module', 'so_feedsagent') . '/css/so_feedsagent_logs.css');

    if (!empty($log['data']['success'])) {

        if ($log['data']['item_timestamp'] != 0) {
            $element['current_state'] = array(
                '#markup' => "",
                '#prefix' => "<div>",
                '#suffix' => "</div>",
            );

            if ($log['data']['success']['timestamp'] < $log['data']['item_timestamp']) {
                $element['current_state']['#markup'] = t("Data are out of date !");
                $element['current_state']['#prefix'] = "<div class='sfa_alert'>";
            }
        }

        $dt = DateTime::createFromFormat('U', $log['data']['success']['timestamp']);
        $dt->setTimezone(new DateTimeZone('Europe/Paris'));

        $element['success'] = array(
            '#markup' => t("The last successful update took place on @date at @time @type.", array(
                '@date' => $dt->format('d/m/Y'),
                '@time' => $dt->format('H:i:s'),
                '@type' => !empty($log['data']['success']['operation']) ? "(" . $log['data']['success']['operation'] . ")" : "",
            )),
            '#prefix' => "<div class='sfa_success_section sfa_section'>",
            '#suffix' => "</div>",
        );
    } else {
        $element['success'] = array(
            '#markup' => t("No successful item update has been recorded yet."),
            '#prefix' => "<div class='sfa_success_section sfa_section'>",
            '#suffix' => "</div>",
        );
    }

    if (!empty($log['data']['fail'])) {

        $element['fail'] = array(
            '#theme' => 'item_list',
            '#type' => 'ul',
            '#title' => "Since last successful update, one or more errors have occured" . " : ",
            '#items' => array(),
            '#prefix' => "<div class='sfa_fail_section sfa_section'>",
            '#suffix' => "</div>",
        );

        foreach ($log['data']['fail'] as $fail) {
            $dt = DateTime::createFromFormat('U', $fail['timestamp']);
            $dt->setTimezone(new DateTimeZone('Europe/Paris'));

            $element['fail']['#items'][] = t("On @date at @time @type", array(
                        '@date' => $dt->format('d/m/Y'),
                        '@time' => $dt->format('H:i:s'),
                        '@type' => !empty($fail['operation']) ? "(" . $fail['operation'] . ")" : "",
                    )) . " : <br />" . $fail['data'];
        }
    }

    if (!empty($log['data']['warning'])) {

        $element['warning'] = array(
            '#theme' => 'item_list',
            '#type' => 'ul',
            '#title' => t("During last successful update, one or more minor problems have been detected") . " : ",
            '#items' => array(),
            '#prefix' => "<div class='sfa_warning_section sfa_section'>",
            '#suffix' => "</div>",
        );

        foreach ($log['data']['warning'] as $warning) {
            $dt = DateTime::createFromFormat('U', $warning['timestamp']);
            $dt->setTimezone(new DateTimeZone('Europe/Paris'));

            $element['warning']['#items'][] = t("On @date at @time @type", array(
                        '@date' => $dt->format('d/m/Y'),
                        '@time' => $dt->format('H:i:s'),
                        '@type' => !empty($warning['operation']) ? "(" . $warning['operation'] . ")" : "",
                    )) . " : <br />" . $warning['data'];
        }
    }

    $diary_items = array();

    if (!empty($log['data']['info'])) {

        foreach ($log['data']['info'] as $timestamp => $info) {
            $dt = DateTime::createFromFormat('U', $info['timestamp']);
            $dt->setTimezone(new DateTimeZone('Europe/Paris'));

            $diary_items[$timestamp] = array(
                'data' => t("On @date at @time @type", array(
                    '@date' => $dt->format('d/m/Y'),
                    '@time' => $dt->format('H:i:s'),
                    '@type' => !empty($info['operation']) ? "(" . $info['operation'] . ")" : "",
                )) . " : <br />" . $info['data'],
                'class' => array('sfa_gravity_' . $info['gravity']),
            );
        }
    }

    if (!empty($log['data']['state'])) {

        foreach ($log['data']['state'] as $timestamp => $info) {
            $dt = DateTime::createFromFormat('U', $info['timestamp']);
            $dt->setTimezone(new DateTimeZone('Europe/Paris'));

            $diary_items[$timestamp] = array(
                t("On @date at @time @type", array(
                    '@date' => $dt->format('d/m/Y'),
                    '@time' => $dt->format('H:i:s'),
                    '@type' => !empty($info['operation']) ? "(" . $info['operation'] . ")" : "",
                )) . " : <br />" . $info['data'],
                'class' => array('sfa_gravity_2'),
            );
        }
    }



    if (!empty($diary_items)) {

        ksort($diary_items);

        $element['diary'] = array(
            '#theme' => 'item_list',
            '#title' => t("Node's diary"),
            '#type' => 'ul',
            '#items' => $diary_items,
            '#prefix' => "<div class='sfa_diary_section sfa_section'>",
            '#suffix' => "</div>",
        );
    }
}

function so_feedsagent_log_item_display($item_id, $nid = null) {

    $logs = array();
    $output = array(
        'content' => array(),
        'back' => isset($_GET['destination']) ? array('#markup' => "<div>" . l(t("Back"), $_GET['destination']) . "</div>") : null,
    );

    if($nid == null) {

        $results = db_query(
            "SELECT l.id, l.container_id, l.data, n.nid, n.title, n.type, n.language FROM {feedsagent_logs} AS l
             LEFT JOIN node AS n ON n.nid=l.container_id
             WHERE l.object_id=?
             ORDER BY id ASC",
            array($item_id)
        );

    } else {

        $results = db_query(
            "SELECT l.id, l.container_id, n.title, n.type, n.language FROM {feedsagent_logs} AS l
             LEFT JOIN node AS n ON n.nid=l.container_id
             WHERE l.object_id=? AND l.container_id=?
             ORDER BY id ASC",
            array($item_id, $nid)
        );
    }

    while($row = $results->fetchObject()) {

        if($row->nid == null) {
            $data = unserialize($row->data);
            $archived_infos = $data['identification'];

        } else {
            $archived_infos = null;
        }

        $logs[] = array(
            'id' => $row->id,
            'item_id' => $item_id,
            'nid' => $row->container_id,
            'node_title' => $row->title,
            'node_type' => $row->type,
            'node_language' => $row->language,
            'archived_infos' => $archived_infos,
        );
    }

    if(count($logs) == 0) {

        $output['content'] =  array(
            'no_result' => array(
                '#attached' => array('css' => array(drupal_get_path('module', 'so_feedsagent') . '/css/so_feedsagent_admin.css')),
                '#markup' => "<div class='overview_empty_msg'>- " . t("No log found") . " -</div>",
            ),
        );

    } elseif(count($logs) == 1) {

        $node = node_load($logs[0]['nid']);

        if(!empty($node)) {
            $output['content'] = so_feedsagent_importer_single_node_update($node);
        } else {
            drupal_set_title(t("Logs of item '@item' while was matching node '@nid'", array('@item' => $item_id, '@nid' => $logs[0]['nid'])));
            so_feedsagent_log_item_build($output['content'], $item_id, $logs[0]['nid']);
        }

    } else {

        drupal_set_title(t("Logs of item '@item'", array('@item' => $item_id)));

        $nodes_types = node_type_get_names();

        $rows = array();

        foreach($logs as $log) {

            $rows[] = array(
                $log['nid'],
                (
                    !empty($log['node_title']) ?
                    l($log['node_title'], drupal_get_path_alias('node/' . $log['nid'], $log['node_language']), array('attributes' => array('target' => '_blank'))) :
                    "<em>" . t("non existent node") . "</em>"
                ),
                (
                    !empty($log['node_type']) ?
                    $nodes_types[$log['node_type']] :
                    "-"
                ),
                (
                    !empty($log['node_language']) ?
                    $log['node_language'] :
                    "-"
                ),
                (
                    !empty($log['archived_infos']) ?
                    $log['archived_infos'] :
                    ""
                ),
                l(t("See logs"), 'admin/reports/feedsagent/logs/item/' . $item_id . '/' . $log['nid'], array('attributes' => array('target' => '_blank'))),
            );
        }

        $output['content'] = array(
            '#theme' => 'table',
            '#header' => array(
                "NID",
                t("Title"),
                t("Type"),
                t("Language"),
                t("Archived infos"),
                t("Action"),
            ),
            '#rows' => $rows,
        );
    }

    return $output;
}

function so_feedsagent_report_items() {

    $output = array();

    $output[] = drupal_get_form('so_feedsagent_report_items_log_search_form');

    return $output;
}

function so_feedsagent_report_items_log_search_form($form, &$form_state) {

    return array(
        'item_log_search' => array(
            '#type' => 'fieldset',
            '#title' => t("Search item logs"),

            'item_id' => array(
                '#type' => 'textfield',
                '#title' => t("Item ID"),
            ),
            'search' => array(
                '#type' => 'submit',
                '#value' => t("Search"),
            ),
        ),
    );
}

function so_feedsagent_report_items_log_search_form_submit($form, &$form_state) {

    $item_id = $form_state['values']['item_id'];

    if(empty($item_id)) {
        drupal_set_message(t("You must enter an item ID"), 'warning');
        return;
    }

    drupal_goto('admin/reports/feedsagent/logs/item/' . trim($item_id), array('query' => drupal_get_destination()));
}

function _so_feedsagent_importer_error_handler($error_level, $message, $filename, $line, $context) {

    if (array_key_exists('error_handler_params', $context)) {
        $item_infos = array(
            'id' => $context['error_handler_params']['item_id'],
            'container' => $context['error_handler_params']['nid'],
        );

        $event_infos = array(
            'data' => $message . " : <br />" . $context['error_handler_params']['data'],
        );

        so_feedsagent_log_item_write('warning', $item_infos, $event_infos);

        // DEV : provisoire, en attendant la mise en place des récaps de logs
        watchdog('Importer', $message . "<br />Context : <br /><pre>@context</pre>", array('@context' => print_r($context['error_handler_params'], true)), WATCHDOG_ERROR);

    } else {
        _drupal_error_handler($error_level, $message, $filename, $line, $context);
    }

    return true; // Ne pas exécuter le gestionnaire interne de PHP
}

function _so_feedsagent_parse_exception($exception) {

    if ($exception instanceof PDOException) {

        return t("Error") . " : " . $exception->errorInfo[2] .
                "<br />" .
                t("Query") . " : " . $exception->query_string .
                (!empty($exception->args) ? "<br />" . t("Arguments") . " : " . implode(' | ', $exception->args) : "");

    } elseif ($exception instanceof SfaImporterException) {

        return t("Error") . " : " . $exception->message;

    } else {
        _drupal_exception_handler($exception);
        return t("Unknown error. See watchdog for further details.");
    }
}

function so_feedsagent_log_item_read($item_id, $nid) {

    $log = db_query("SELECT * FROM {feedsagent_logs} WHERE type=? AND object_id=? AND (container_id=? OR container_id=0)", array(
        'item',
        $item_id,
        $nid,
            ))->fetchAssoc();

    if (!empty($log)) {
        $log['data'] = unserialize($log['data']);
    } else {
        $log = array(
            'type' => 'item',
            'object_id' => $item_id,
            'container_id' => (int) $nid,
            'state' => 0,
            'data' => array(
                'item_timestamp' => 0,
                'success' => array(),
                'fail' => array(),
                'warning' => array(),
                'infos' => array(),
                'state' => array(),
                'state_blacklist' => array(),
            ),
        );
    }

    return $log;
}

/**
 * Write log informations about result of various operations performed on items
 *
 * @param string $log_type : either 'identification', 'success', 'fail', 'warning', 'info', 'state'
 * @param array $item_infos :
 *                      - 'id' : string : item's id.
 *                      - 'container' : string : item's container'id (currently, it can be only a nid in case of feeds' items)
 *                      - 'timestamp' : int : optional : item timestamp in the context of its feed. It's used to determine
 *                                            if the stored one is up or out to date.
 *
 * @param array $event_infos : event infos (depend on type of log) :
 *                      ++IDENTIFICATION++ : array :
 *                          - 'infos' : a human readable string, for identifying log entry further. Especially if the log
 *                                      subject disapear (as a node deletion for wich JOIN with node table won't be possible anymore)
 *
 *                      ++SUCCESS++ : array :
 *                          - 'operation' : string : localized string describing the successful performed operation.
 *                          - 'start_timestamp' : int : the starting time of the successful operation. It's used to clean
 *                                                      warnings which has happened during previous successful operation.
 *
 *                      ++FAIL++ : array of arrays :
 *                          - 'operation' : string : localized string describing the failed performed operation.
 *                          - 'data' : string : error details.
 *
 *                      ++WARNING++ : array of arrays :
 *                          - 'operation' : string : localized string describing the performed operation.
 *                          - 'data' : string : warning details.
 *
 *                      ++INFO++ : array indexed by timestamp
 *                          - 'operation' : string : localized string describing the performed operation.
 *                          - 'data' : string : info details.
 *                          - 'gravity' : int : 0 = no importance, 1 = good, 2 = needs to be outlined, 3 = bad
 *
 *                      ++STATE++ : array indexed by timestamp
 *                          - 'operation' : string : localized string describing the performed operation.
 *                          - 'data' : string : state details.
 *                          - 'once' : boolean : if set to 'true', this event will be logged one time and will no longer be in the future,
 *                                               until 'reset' is set to 'true'.
 *                          - 'reset' : boolean : if set to 'true', 'once' events list is reseted.
 *
 */
function so_feedsagent_log_item_write($log_type, $item_infos, $event_infos) {

    $settings = variable_get('feedsagent_settings', array());

    $log = so_feedsagent_log_item_read($item_infos['id'], $item_infos['container']);

    if (array_key_exists('timestamp', $item_infos)) {
        $log['data']['item_timestamp'] = $item_infos['timestamp'];
    }

    global $user;
    $event_infos['operation'] .= !empty($user->name) ? " " . t("by @user", array('@user' => ucfirst($user->name))) : "";

    switch ($log_type) {

        case 'identification':
            $log['data']['identification'] = $event_infos['infos'];
            break;

        case 'success':
            $log['state'] = 0;
            $log['data']['fail'] = array();

            $log['data']['success'] = array(
                'operation' => $event_infos['operation'],
                'timestamp' => time(),
            );

            foreach ($log['data']['warning'] as $key => $warning) {
                if ($warning['timestamp'] < $event_infos['start_timestamp']) {
                    unset($log['data']['warning'][$key]);
                }
            }

            $log['data']['info'][$log['data']['success']['timestamp']] = array(
                'operation' => $event_infos['operation'],
                'data' => t("Node was successfully updated"),
                'timestamp' => $log['data']['success']['timestamp'],
                'gravity' => 1,
            );

            break;

        case 'fail':
            $log['state']++;

            $timestamp = time();

            $log['data']['fail'][] = array(
                'operation' => $event_infos['operation'],
                'data' => $event_infos['data'],
                'timestamp' => $timestamp,
            );

            $log['data']['info'][$timestamp] = array(
                'operation' => $event_infos['operation'],
                'data' => t("Update failed"),
                'timestamp' => $timestamp,
                'gravity' => 3
            );

            $log['data']['fail'] = array_slice($log['data']['fail'], -10, null, true);

            break;

        case 'warning':
            $log['data']['warning'][] = array(
                'operation' => $event_infos['operation'],
                'data' => $event_infos['data'],
                'timestamp' => time(),
            );

            $log['data']['warning'] = array_slice($log['data']['warning'], -10, null, true);

            break;

        case 'info':
            $timestamp = time();

            $log['data']['info'][$timestamp] = array(
                'operation' => $event_infos['operation'],
                'data' => $event_infos['data'],
                'timestamp' => $timestamp,
                'gravity' => (int) $event_infos['gravity'],
            );

            $num_diary_entries = array_key_exists('num_diary_entries', $settings['importer']['logs']) ? $settings['importer']['logs']['num_diary_entries'] : 10;
            $log['data']['info'] = array_slice($log['data']['info'], -$num_diary_entries, null, true);

            break;

        case 'state':

            if(array_key_exists('reset', $event_infos) && $event_infos['reset'] == true) {
                $log['data']['state_blacklist'] = array();
            }

            if(array_key_exists('once', $event_infos) && $event_infos['once'] == true) {

                $black_list_key = md5($event_infos['operation'] . $event_infos['data']);

                if(array_key_exists($black_list_key, $log['data']['state_blacklist'])) {
                    break;
                }

                $log['data']['state_blacklist'][$black_list_key] = true;
            }

            $timestamp = time();

            $log['data']['state'][$timestamp] = array(
                'operation' => $event_infos['operation'],
                'data' => $event_infos['data'],
                'timestamp' => $timestamp,
            );

            break;
    }

    // TODO : cette opération devrait être placée en début de "case 'state':", seulement, VV a besoin d'un peu de nettoyage pour le moment. (07/11/2014)
    if(count($log['data']['state']) > 19) {
        $cleaned_state_array = array_slice($log['data']['state'], 0, 10, true); // les 10 premiers
        $cleaned_state_array += array_slice($log['data']['state'], -9, null, true); // les 9 derniers
        $log['data']['state'] = $cleaned_state_array;
    }

    $log['state'] = min($log['state'], 255);

    $log['data'] = serialize($log['data']);

    $db_key = array_key_exists('id', $log) ? 'id' : array();

    drupal_write_record('feedsagent_logs', $log, $db_key);
}

function so_feedsagent_report_summary() {

    require_once DRUPAL_ROOT . '/includes/batch.inc';

    $output = array();

    $queues_results = db_query("SELECT DISTINCT name FROM {queue} WHERE name LIKE 'drupal_batch%'");

    $running_ops = array();

    while($queue_row = $queues_results->fetchObject()) {

        $parsed_queue_name = explode(':', $queue_row->name);

        $batch = unserialize(db_query("SELECT batch FROM {batch} WHERE bid=?", array($parsed_queue_name[1]))->fetchColumn());

        if(!empty($batch) && array_key_exists('feedsagent_feed_id', $batch['sets'][$batch['current_set']])) {

            $batch_loaded = batch_load($batch['id']);
            $batch_is_active = !empty($batch_loaded) ? true : false;

            $running_ops[] = array(
                'title' => $batch['sets'][$batch['current_set']]['title'] .
                           " - " .
                           t("by @username", array('@username' => $batch['sets'][$batch['current_set']]['feedsagent_user_name'])),

                'state' => $batch_is_active == true ? t("Active") : t("Inactive"),

                'started' => DateTime::createFromFormat('U', floor($batch['sets'][$batch['current_set']]['start']))
                            ->setTimezone(new DateTimeZone('Europe/Paris'))
                            ->format('d/m/Y H:i:s'),

                'elapsed' => floor($batch['sets'][$batch['current_set']]['elapsed'] / 1000),

                'processed' => "item " .
                                ($batch['sets'][$batch['current_set']]['total'] - $batch['sets'][$batch['current_set']]['count']) .
                                "/" .
                                $batch['sets'][$batch['current_set']]['total'],

                'operations' => l(t("Restart"), 'admin/reports/feedsagent/batch/restart/' . $batch['id'] . '/' . (int)$batch_is_active, array(
                                    'attributes' => array(
                                        'target' => '_blank',
                                    ),
                                )) .
                                "<br />" .
                                l(t("Cancel"), 'admin/reports/feedsagent/batch/cancel/' . $batch['id']),
            );
        }
    }

    $output['batch_activity'] = array(
        '#type' => 'fieldset',
        '#title' => t("Batches' current activity"),
        '#description' => t("Batch's elapsed time is updated each time a new item is processed.") .
                          " " .
                          t("If values remain unchanged during a long time, that probably means the operation has crashed.") .
                          "<br />" .
                          t("Using 'restart' operation while a batch is already running starts a parallel batch (two more items processed in the same time).") .
                          " " .
                          t("That's not a real problem (if server's CPU is ok), but when second batch will finish, the first one will be frozen (but all items will have been processed)."),

        'content' => array(
            '#theme' => 'table',
            '#header' => array(
                t("Current operation"),
                t("State"),
                t("Started"),
                t("Elapsed time"),
                t("Processed", array('@sfa_item' => null)),
                t("Operations")
            ),
            '#rows' => $running_ops,
            '#empty' => t("Currently, no activity has been detected."),
        ),
    );




    return $output;
}

function so_feedsagent_importer_batch_restart($batch_id, $is_active = false) {

    if($is_active == false) {
        $new_token = drupal_get_token($batch_id);
        db_query("UPDATE {batch} SET token=? WHERE bid=?", array($new_token, $batch_id));
    }

    drupal_goto('batch', array(
        'query' => array(
            'op' => 'start',
            'id' => $batch_id,
        ),
    ));
}

function so_feedsagent_importer_batch_cancel($batch_id) {

    db_query("DELETE FROM {queue} WHERE name LIKE ?", array('drupal_batch:' . $batch_id . ':%'));
    db_query("DELETE FROM {batch} WHERE bid=?", array($batch_id));

    drupal_set_message(t("Batch has been canceled"));
    drupal_goto('admin/reports/feedsagent');
}

/**
 * A format callback : single or multiple type 'value' for most fields types
 *
 * @param array $field
 * @param array $previous_values
 * @param array $widgets_options
 * @param int $nid
 * @param string $item_id
 *
 * @return array
 */
function _so_feedsagent_value_format_callback($field, $previous_values, $widgets_options, $nid, $item_id) {

    $field_value = array();

    foreach ((array) $field['values'] as $value) {

        // TODO : A SUPPRIMER QUAND LES FORMATEURS DE CHAMPS D'IMPORT SERONT EN PLACE
        if ($field['type'] == 'integer') {
            $value = round($value);
        } elseif($field['type'] == 'decimal') {
            $value = str_replace(',', '.', $value);
        } elseif($field['type'] == 'onoff') {
            $value = (int)(boolean)$value;
        }

        $field_value[] = array('value' => $value);

        if ($field['cardinality'] == 1) {
            break;
        }
    }

    return $field_value;
}

/**
 * A format callback : single or multiple type 'value' for text : it adds the 'full HTML' filter
 *
 * @param array $field
 * @param array $previous_values
 * @param array $widgets_options
 * @param int $nid
 * @param string $item_id
 *
 * @return array
 */
function _so_feedsagent_text_format_callback($field, $previous_values, $widgets_options, $nid, $item_id) {

    $field_value = array();

    foreach ((array) $field['values'] as $value) {

        $field_value[] = array(
            'value' => $value,
            'format' => 'full_html',
        );

        if ($field['cardinality'] == 1) {
            break;
        }
    }

    return $field_value;
}

/**
 * A format callback : write raw values which have been defined in a script or a filter
 *
 * @param array $field
 * @param array $previous_values
 * @param array $widgets_options
 * @param int $nid
 * @param string $item_id
 *
 * @return array
 */
function _so_feedsagent_custom_format_callback($field, $previous_values, $widgets_options, $nid, $item_id) {

    return $field['values'];
}

/**
 * A format callback : type 'value'/'value2' for DateField
 *
 * @param array $field
 * @param array $previous_values
 * @param array $widgets_options
 * @param int $nid
 * @param string $item_id
 *
 * @return array
 */
function _so_feedsagent_2values_format_callback($field, $previous_values, $widgets_options, $nid, $item_id) {

    $field_value = array();

    foreach ((array) $field['values'] as $value) {

        if (is_array($value) == true) {

            $field_value[] = array(
                'value' => $value[0],
                'value2' => $value[1],
            );
        } else {

            $field_value[] = array(
                'value' => $value,
                'value2' => $value,
            );
        }

        if ($field['cardinality'] == 1) {
            break;
        }
    }

    return $field_value;
}

/**
 * A format callback : type 'fid' for FileField/ImageField
 * The callback also recover the file and store it locally.
 *
 * @param array $field
 * @param array $previous_values
 * @param array $widgets_options
 * @param int $nid
 * @param string $item_id
 *
 * @return array
 */
function _so_feedsagent_file_format_callback($field, $previous_values, $widgets_options, $nid, $item_id) {

    $field_value = array();

    $path = 'public://' . $widgets_options[$field['type']]['folder'];

    foreach ((array)$field['values'] as $raw_value) {

        $value = is_array($raw_value) ? $raw_value['file'] : $raw_value;

        $dirname = pathinfo($value, PATHINFO_DIRNAME);
        $drupal_parsed = drupal_parse_url($value);
        $filename = pathinfo($drupal_parsed['path'], PATHINFO_BASENAME);
        $params = parse_url($value, PHP_URL_QUERY);
        $params = (!empty($params) ? '?' : '') . $params;

        $url = $dirname . '/' . rawurlencode($filename) . $params;

        if(strpos($dirname, 'http://') === false
            && strpos($dirname, 'https://') === false
            && strpos($dirname, 'public://') === false) {

            $url = 'http://' . $url;
        }

        $local_filename = preg_replace('#^field#', $nid, $field['field']) . '_' . $filename;

        if(strpos($dirname, 'public://') === 0) {
            $handle = fopen(drupal_realpath($url), 'r');
            $file_content = fread($handle, filesize(drupal_realpath($url)));
            $file = file_save_data($file_content, $path . '/' . $local_filename, FILE_EXISTS_REPLACE);
        } else {
            $file = system_retrieve_file($url, $path . '/' . $local_filename, true, FILE_EXISTS_REPLACE);
        }

        if ($file != false) {
            // l'ajout/suppression des usages est automatiquement effectué par Fields

            $file->display = 1; // spécifique aux fichiers

            $file->description = is_array($raw_value) ? $raw_value['description'] : '';
            $file->title = is_array($raw_value) ? $raw_value['title'] : '';

            $field_value[] = (array)$file; // transtypage obligatoire
        } else {
            $error_handler_params = array(// passé dans le contexte du error_handler
                'nid' => $nid,
                'item_id' => $item_id,
                'data' => "<a href='" . $url . "' target='_blank'>" . $url . "</a>",
            );
            trigger_error("Unavailable file", E_USER_WARNING);
        }

        if ($field['cardinality'] == 1) {
            break;
        }
    }

    return $field_value;
}

function so_feedsagent_importer_batch_missing_items($nid, $item_id, $op, &$context) {

    $batch = batch_get();
    $configuration = $batch['sets'][$batch['current_set']]['configuration'];

    $item_infos = array(
        'id' => $item_id,
        'container' => $nid,
    );

    $event_infos = array(
        'operation' => t("@op of feed '@feed'", array('@op' => $op, '@feed' => $configuration['feed_definition']->label)),
        'once' => true,
    );

    switch ($configuration['feed_definition']->params['importer']['behaviors']['nodes']['obsolete']) {

        case 'none':
            $context['results']['nb_untouched']++;
            $event_infos_action = t("none");

            break;

        case 'relegate':
            db_query(
                "UPDATE {feedsagent_contents} SET weight=100000 WHERE nid=? AND feed_id=? AND item_id=?",
                array(
                    $nid,
                    $configuration['feed_definition']->id,
                    $item_id
                )
            );

            $context['results']['nb_relegated']++;
            $event_infos_action = t("node was relegated at bottom of listing");

            break;

        case 'seo_unpublish':
        case 'unpublish':

            // un node peut être référencé dans plusieurs feeds.
            // si c'est le cas, on ne le dépublie pas : on le déréférence simplement du feed en cours
            $count = db_query("SELECT COUNT(*) AS num FROM {feedsagent_contents} WHERE nid=?", array($nid))->fetchObject()->num;

            if ($count > 1) {
                db_query("DELETE FROM {feedsagent_contents} WHERE nid=? AND feed_id=?", array($nid, $configuration['feed_definition']->id));
                $context['results']['nb_removed']++;
                $event_infos_action = t("since node was existing in several feed, no action was performed");
            } else {
                $status = db_query("SELECT status FROM {node} WHERE nid=?", array($nid))->fetchColumn();

                // si le node n'a pas déjà été dépublié. (économie de logs)
                if ($status == 1) {

                    $node_infos = db_query("SELECT vid, type FROM {node} WHERE nid=?", array($nid))->fetchObject();

                    // phase SEO
                    if($configuration['feed_definition']->params['importer']['behaviors']['nodes']['obsolete'] == 'seo_unpublish'
                            && metatag_entity_supports_metatags('node', $node_infos->type)) {

                        $obsolete_since = db_query(
                            "SELECT obsolete_since FROM {feedsagent_contents} WHERE nid=? AND feed_id=?",
                            array(
                                $nid,
                                $configuration['feed_definition']->id,
                            )
                        )->fetchColumn();

                        $now = time();

                        // le node n'est pas encore marqué comme obsolète
                        if($obsolete_since == 0) {

                            db_query(
                                "UPDATE {feedsagent_contents} SET obsolete_since=? WHERE nid=? AND feed_id=?",
                                array(
                                    $now,
                                    $nid,
                                    $configuration['feed_definition']->id,
                                )
                            );

                            $language = $configuration['feed_definition']->language;

                            so_feedsagent_seo_indexing('desactivate', $nid, $node_infos->vid, $language);

                            $event_infos_action = t("node unpublish was scheduled");
                            $context['results']['nb_seo_unpublished']++;

                            break;

                        // le node est déjà marqué comme étant obsolète, mais depuis moins de 30 jours
                        } elseif($now - $obsolete_since < 2592000 ) { // 3600 * 24 * 30
                            break;
                        }
                    }

                    // statut du node mais également de sa révision en cours
                    db_query("UPDATE {node} AS n LEFT JOIN {node_revision} as r ON r.nid=n.nid SET n.status=0, r.status=0 WHERE n.nid=?", array($nid));

                    // à noter que l'on conserve quand-même le node inscrit dans les contenus SFA, en prévision de sa réapparition possible.

                    $event_infos_action = t("node was unpublished");
                    $context['results']['nb_unpublished']++;
                }
            }

            break;

        case 'seo_delete':
        case 'delete':
            // un node peut être référencé dans plusieurs feeds.
            // si c'est le cas, on ne le supprime pas : on le déréférence simplement du feed en cours
            $count = db_query("SELECT COUNT(*) AS num FROM {feedsagent_contents} WHERE nid=?", array($nid))->fetchObject()->num;

            if ($count > 1) {
                db_query("DELETE FROM {feedsagent_contents} WHERE nid=? AND feed_id=?", array($nid, $configuration['feed_definition']->id));
                $context['results']['nb_removed']++;
                $event_infos_action = t("since node was existing in several feed, no action was performed");
            } else {

                $node_infos = db_query("SELECT vid, type FROM {node} WHERE nid=?", array($nid))->fetchObject();

                // phase SEO
                if($configuration['feed_definition']->params['importer']['behaviors']['nodes']['obsolete'] == 'seo_delete'
                        && metatag_entity_supports_metatags('node', $node_infos->type)) {

                    $obsolete_since = db_query(
                        "SELECT obsolete_since FROM {feedsagent_contents} WHERE nid=? AND feed_id=?",
                        array(
                            $nid,
                            $configuration['feed_definition']->id,
                        )
                    )->fetchColumn();

                    $now = time();

                    // le node n'est pas encore marqué comme obsolète
                    if($obsolete_since == 0) {

                        db_query(
                            "UPDATE {feedsagent_contents} SET obsolete_since=? WHERE nid=? AND feed_id=?",
                            array(
                                $now,
                                $nid,
                                $configuration['feed_definition']->id,
                            )
                        );

                        $language = $configuration['feed_definition']->language;

                        so_feedsagent_seo_indexing('desactivate', $nid, $node_infos->vid, $language);

                        $event_infos_action = t("node deletion was scheduled");
                        $context['results']['nb_seo_deleted']++;

                        break;

                    // le node est déjà marqué comme étant obsolète, mais depuis moins de 30 jours
                    } elseif($now - $obsolete_since < 2592000 ) { // 3600 * 24 * 30
                        break;
                    }
                }

                global $user;
                $uid_backup = $user->uid;
                $user->uid = 1; // les anonymous n'ont pas les droits

                node_delete($nid);
                $context['results']['nb_deleted']++;
                $event_infos_action = t("node was deleted");

                $user->uid = $uid_backup;
            }

            break;
    }

    if (!empty($event_infos_action)) {
        $event_infos['data'] = t("Item was no longer exist in feed.") . " ". t("Action taken") . " : " . $event_infos_action;
        so_feedsagent_log_item_write('state', $item_infos, $event_infos);
    }
}

/*
 * Update node's meta ROBOTS.
 * Be careful ! There is no verification on metatag support of node type !
 *
 * @params $activate : boolean : set on 'true' (default) activate robots indexation.
 *                               Set on 'false', prevents robots indexation
 */
function so_feedsagent_seo_indexing($op, $nid, $vid, $language) {

    $metatags_array = metatag_metatags_load_multiple('node', array($nid));

    if(empty($metatags_array) || !array_key_exists($nid, $metatags_array)) {

        $metatags_array[$nid] = array(
            $nid => array(
                $language => array(),
            ),
        );
    }

    $metatags = reset($metatags_array[$nid]);

    if($op == 'desactivate') {
        $robots = array(
            'index' => 0,                       // Allow search engines to index this page
            'noindex' => 'noindex',             // Prevents search engines from indexing this page
            'follow' => 'follow',               // Allow search engines to follow links on this page
            'nofollow' => 0,                    // Prevents search engines from following links on this page
            'noydir' => 'noydir',               // Prevents Yahoo! from listing this page in the Yahoo! Directory
            'noimageindex' => 'noimageindex',   // Prevent search engines from indexing images on this page
        );
    } else {
        $robots = array(
            'index' => 'index',                 // Allow search engines to index this page
            'noindex' => 0,                     // Prevents search engines from indexing this page
            'follow' => 'follow',               // Allow search engines to follow links on this page
            'nofollow' => 0,                    // Prevents search engines from following links on this page
            'noydir' => 0,                      // Prevents Yahoo! from listing this page in the Yahoo! Directory
            'noimageindex' => 0,                // Prevent search engines from indexing images on this page
        );
    }

    if(!array_key_exists('robots', $metatags[$language])) {
        $metatags[$language]['robots'] = array('value' => array());
    }

    $metatags[$language]['robots']['value'] = array_merge($metatags[$language]['robots']['value'], $robots);

    metatag_metatags_save('node', $nid, $vid, $metatags, $language);
}

function so_feedsagent_importer_batch_finished($success, $results, $operations) {

    $batch = batch_get();
    $configuration = $batch['sets'][$batch['current_set']]['configuration'];

    $is_cron = ($batch['op'] == 'cron' || $batch['op'] == 'cron_import');

    if ($success != true && $is_cron != true) { // DEV : logs
        $error_operation = reset($operations);
        drupal_set_message(t("An unknown error occurred while processing item : '@item'.<br />Reference of the batch : '@batchid'", array(
                    '@item' => $error_operation[1][0]['id'],
                    '@batchid' => $batch['id'],
                        )
                ), 'error');

        if (module_exists('devel')) {
            dpm($error_operation, "Debug infos");
        }
    }

    $message_queue = array();

    if (!empty($configuration['batch_messages'])) {
        foreach ($configuration['batch_messages'] as $message) {
            $message_queue[] = $message;
        }
    }

    if (!empty($results['nb_created'])) {
        $message_queue[] = format_plural($results['nb_created'], "1 node created", "@count nodes created");
    }

    if (!empty($results['nb_up_to_date'])) {
        $message_queue[] = format_plural($results['nb_up_to_date'], "1 node was up to date", "@count nodes were up to date");
    }

    if (!empty($results['nb_updated'])) {
        $message_queue[] = format_plural($results['nb_updated'], "1 node was updated", "@count nodes were updated");
    }

    if (!empty($results['nb_imported'])) {
        $message_queue[] = format_plural($results['nb_imported'], "1 node was (re)imported", "@count nodes were (re)imported");
    }

    if (!empty($results['nb_untouched'])) {
        $message_queue[] = format_plural($results['nb_untouched'], "1 obsolete node was left untouched", "@count obsolete nodes were left untouched");
    }

    if (!empty($results['nb_relegated'])) {
        $message_queue[] = format_plural($results['nb_relegated'], "1 obsolete node was relegated", "@count obsolete nodes were relegated");
    }

    if (!empty($results['nb_removed'])) {
        $message_queue[] = format_plural($results['nb_removed'], "1 obsolete node was removed from feed", "@count obsolete nodes were removed from feed");
    }

    if (!empty($results['nb_unpublished'])) {
        $message_queue[] = format_plural($results['nb_unpublished'], "1 obsolete node was unpublished", "@count obsolete nodes were unpublished");
    }

    if (!empty($results['nb_seo_unpublished'])) {
        $message_queue[] = format_plural($results['nb_seo_unpublished'], "1 obsolete node unpublication was scheduled", "@count obsolete nodes unpublications were scheduled");
    }

    if (!empty($results['nb_deleted'])) {
        $message_queue[] = format_plural($results['nb_deleted'], "1 obsolete node was deleted", "@count obsolete nodes were deleted");
    }

    if (!empty($results['nb_seo_deleted'])) {
        $message_queue[] = format_plural($results['nb_seo_deleted'], "1 obsolete node deletion was scheduled", "@count obsolete nodes deletions were scheduled");
    }

    if ($is_cron != true) {
        drupal_set_message(implode('<br />', $message_queue));
    } else {
        watchdog('Importer', $batch['title'] . " : <br />" . implode('<br />', $message_queue));
    }

    return;
}

function so_feedsagent_importer_get_periodicities() {
    return array(
        'hourly' => t("Hourly@fai", array('@fai' => null)),
        'daily' => t("Daily@fai", array('@fai' => null)),
        'weekly' => t("Weekly@fai", array('@fai' => null)),
        'monthly' => t("Monthly@fai", array('@fai' => null)),
    );
}

function theme_so_feedsagent_new_feed($vars) {

    $output = "";

    $element = $vars['element'];

    $rows = array(
        array(
            render($element['label']),
            render($element['language']),
            render($element['connector']),
            render($element['pattern']),
            render($element['add']),
        ),
    );

    $output .= theme('table', array('header' => null, 'rows' => $rows));

    return $output;
}

function theme_so_feedsagent_feeds_overview($vars) {

    $output = "";

    $element = $vars['element'];

    $rows = array();

    $feeds = element_children($element['feeds']);

    if (empty($feeds)) {
        return "<div class='overview_empty_msg'>- " . t("No feed set yet") . " -</div>";
    }

    $oddeven = -1; // on gère notre propre stripping : celui de D7 n'est pas adapté au 'rowspan'

    foreach ($feeds as $feed) {

        $rows[$feed] = array(
            'data' => array(
                array('data' => render($element['active'][$feed]), 'rowspan' => 2),
                render($element['feeds'][$feed]['label']),
                render($element['feeds'][$feed]['language']),
                render($element['feeds'][$feed]['connector']),
                render($element['feeds'][$feed]['periodicity']),
                render($element['feeds'][$feed]['operations']['edit']) . " "
                . render($element['feeds'][$feed]['operations']['delete']) . " "
                . render($element['feeds'][$feed]['operations']['update']) . " "
                . render($element['feeds'][$feed]['operations']['import']),
                render($element['force_update'][$feed]),
            ),
            'no_striping' => true,
            'class' => array($oddeven == -1 ? 'odd' : 'even'),
        );

        $rows[$feed . '_infos'] = array(
            'data' => array(
                array('data' => render($element['feeds'][$feed]['feed_infos']), 'colspan' => 6),
            ),
            'no_striping' => true,
            'class' => array($oddeven == -1 ? 'odd' : 'even'),
        );

        if (!empty($element['##warnings'])) {
            $rows[$feed]['data'][] = !empty($element['##warnings'][$feed]) ? "<span class='overview_warning'>" . $element['##warnings'][$feed] . "</div>" : "";
            $rows[$feed . '_infos']['data'][] = "";
        }

        $oddeven *= -1;
    }

    $headers = array(
        t("Active"),
        t("Name"),
        t("Language"),
        t("Connector"),
        t("Periodicity"),
        t("Operations"),
        t("Forced"),
    );

    if (!empty($element['##warnings'])) {
        $headers[] = t("Warnings");
    }

    $output .= theme('table', array('header' => $headers, 'rows' => $rows));

    return $output;
}

function theme_so_feedsagent_importer_update($vars) {

    $output = "";

    $element = $vars['element'];

    $rows = array();

    $feeds = element_children($element['feeds']);

    foreach ($feeds as $feed) {

        $rows[$feed] = array(
            render($element['active'][$feed]),
            render($element['feeds'][$feed]['label']),
            render($element['feeds'][$feed]['language']),
            render($element['feeds'][$feed]['connector']),
            render($element['periodicity'][$feed]),
            render($element['feeds'][$feed]['operations']['update']),
        );

        if (!empty($element['##warnings'])) {
            $rows[$feed]['data'][] = !empty($element['##warnings'][$feed]) ? "<span class='overview_warning'>" . $element['##warnings'][$feed] . "</div>" : "";
            $rows[$feed . '_infos']['data'][] = "";
        }
    }

    $headers = array(
        t("Active"),
        t("Name"),
        t("Language"),
        t("Connector"),
        t("Update periodicity"),
        t("Operations"),
    );

    if (!empty($element['##warnings'])) {
        $headers[] = t("Warnings");
    }

    $output .= theme('table', array('header' => $headers, 'rows' => $rows));

    return $output;
}