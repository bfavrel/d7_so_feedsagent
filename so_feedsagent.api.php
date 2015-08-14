<?php

/**
 * Modules can register a connector type.
 *
 * @return array : indexed by 'connector machine name':
 *          - 'label' : human readable name
 *          - 'module' : the module which implement the connector
 *          - 'class' : the connector class (@see _classes/SoFeedsAgentConnectorAbstract.class.php, for further documentation)
 */
function hook_so_feedsagent_connectors() {}

/**
 * Informs modules that a connector is about to be deleted
 *
 * @param stdClass $connector  : the database connector object
 */
function hook_so_feedsagent_connector_delete($connector) {}

/**
 * Informs modules that a feed is about to be deleted
 *
 * @param stdClass $feed : the database feed object
 */
function hook_so_feedsagent_feed_delete($feed) {}

/**
 * Provides operations to the Generator Content Type Wizard.
 *
 * @return array $form_map : indexed by operation machine name :
 *                      - 'label' : string : the label of the operation.
 *                      - 'steps' : array : indexed by numeric (from 1 ; 0 being the mandatory operation selection initial step) :
 *                              - 'title' : string : the step's title.
 *                              - 'help' : advice or precisions for step's element's use.
 *                              - 'function' : string : function to call in 'so_feedsagent_generator_step_callback()' step. If a validation function 
 *                                (same name with '_validation' suffix) or a submission function is implemented (same name with suffix '_submission'), 
 *                                each of them will be automatically call with '$form_state' as parameter.
 *                                Function can optionally return an array of form additions which will be merged (recursively) 
 *                                with the main form ('#attributes', '#attached', etc.)
 *                              - 'method' : string : method to call in 'so_feedsagent_generator_step_callback()' step. If a validation method 
 *                                (same name with 'Validation' suffix) or a submission method is implemented (same name with suffix 'Submission'), 
 *                                each of them will be automatically call with '$form_state' as parameter.
 *                                Method can optionally return an array of form additions which will be merged (recursively) 
 *                                with the main form ('#attributes', '#attached', etc.)
 *                              - 'args' : array : arguments to pass to the function/method. @see each function/method documentation for args details.
 *                              - 'previous_button' : boolean : has this step a previous button ?
 *
 * NOTA : all remaining values in $form_state['values'] will be wrotten in $form_state['wizzard_params'] : @see so_feedsagent_generator_navigation_submit().
 *        The submission functions/methods can also validate data. If any validation/submission functions/methods returns 'false', the form will stay 
 *        in current state, until correction has been made. ACHTUNG ! : since the validation/submission actually happends in a submit function of the form,
 *        the 'form_set_error()' function musn't be used : it will crash the whole form.
 * 
 * @todo : implement our own form_set_error() function.
 * 
 * @see so_feedsagent_generator_form() for the default structure of form map.
 */
function hook_sfa_generator_form_map() {}