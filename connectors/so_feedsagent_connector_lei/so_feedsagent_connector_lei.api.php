<?php

/**
 * Offers to modules opportunity to process LEI values for a given node.
 * This hook is called right after each processing of an
 * XML section (ie : 'Criteres', 'Prestataire', 'Horaires', etc.)
 *
 * @param string $type : currently only 'schedules' (= 'Horaires') is available.
 * @param array $data : section of LEI XML data (converted to array) concerning the current $type.
 * @param array &$fields : {feedsagent_content_types}.fields.
 * @param stdClass $node : the node object currently beeing processed.
 */
function hook_lei_importer_values($section, $data, &$fields, $node) {}