(function ($) {
    $(document).ready(function(){

        //----- FIELDS OVERRIDES -----

        $('div.override_controls').each(function(){
            $(this).parent('div.overridable_field').children().first().before($(this).detach());
            $(this).show();
        });

        $('a.override_on').click(function(evt){
            evt.preventDefault();
            $(this).parents('div.overridable_field').find('input.override_op').val(1);
            $(this).hide();
            $(this).siblings('a.override_off').show();
            $(this).parents('div.overridable_field').toggleClass('overrided_field');
        });

        $('a.override_off').click(function(evt){
            evt.preventDefault();
            $(this).parents('div.overridable_field').find('input.override_op').val(0);
            $(this).hide();
            $(this).siblings('a.override_on').show();
            $(this).parents('div.overridable_field').toggleClass('overrided_field');
        });

        //----- WIZZARD : FEED FIELDS OVERVIEW -----

        Drupal.behaviors.so_feedsagent = {
            attach: function (context, settings) {
                initFeedsagent();
            }
        };

        var initFeedsagent = function(){

            // fields 'select all' checkboxes
            $('form.feedsagent_wizzard th input.select_all_fields').click(function(){
                $(this).parents('table').first().find('tr > td.field_select input.form-checkbox').attr('checked', this.checked);
            });

            // groups' sub-fields autocheck
            $('form.feedsagent_wizzard div.subfields_autocheck td.master_checkbox_cell input:checkbox').click(function(){
                $(this).parents('td.master_checkbox_cell').parent('tr').find('input:checkbox').attr('checked', this.checked);
            });

            // 'in_virtual_type' 'select_all' checkboxes
            $('form.feedsagent_wizzard fieldset.fields_overview_legend input.in_virtual_type_select_all').change(function(){
                $('form.feedsagent_wizzard').find('tr.in_virtual_type > td.field_select input.form-checkbox').attr('checked', this.checked);
            });
            // 'in_custom_selection' 'select_all' checkboxes
            $('form.feedsagent_wizzard fieldset.fields_overview_legend input.in_custom_selection_select_all').change(function(){
                $('form.feedsagent_wizzard').find('tr.in_custom_selection > td.field_select input.form-checkbox').attr('checked', this.checked);
            });
            // 'in_drupal_type' 'select_all' checkboxes
            $('form.feedsagent_wizzard fieldset.fields_overview_legend input.in_drupal_type_select_all').change(function(){
                $('form.feedsagent_wizzard').find('tr.in_drupal_type > td.field_select input.form-checkbox').attr('checked', this.checked);
            });
            // 'in_connector' 'select_all' checkboxes
            $('form.feedsagent_wizzard fieldset.fields_overview_legend input.in_connector_select_all').change(function(){
                $('form.feedsagent_wizzard').find('tr.in_connector > td.field_select input.form-checkbox').attr('checked', this.checked);
            });

            $('form.feedsagent_wizzard.form_autoclean').submit(function(){

                //-- checkboxes
                // non transmises si non-checked

                //-- select
                $(this).find('select option:selected[value=""]').parent().remove();
                
            });
        };

        initFeedsagent();
    });
})(jQuery);