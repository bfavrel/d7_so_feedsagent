<?php

function so_feedsagent_rules_condition_info() {

    return array(
        'so_feedsagent_is_sfa_node' => array(
            'label' => t("The node is an SFA one"),
            'group' => "SO FeedsAgent",
            'arguments' => array(
                'node' => array(
                    'label' => t("Node id (nid)"),//\\
                    'type' => 'node',
                ),
            ),
        ),
    );
}

function so_feedsagent_is_sfa_node($node) {

    $sfa_node = db_query("SELECT nid FROM {feedsagent_contents} WHERE nid=?", array($node->nid))->fetchColumn();
    return !empty($sfa_node);
}