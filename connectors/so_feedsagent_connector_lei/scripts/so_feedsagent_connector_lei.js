(function ($) {
    $(document).ready(function(){
       
        $('a.connector_lei_see_xml_link').click(function(evt){
                       
            if($(this).data('baseUrl') == undefined) {
                $(this).data('baseUrl', $(this).attr('href'));
            }
            
            var rfrom = $(this).parent('div.connector_lei_see_xml').find('input.connector_lei_see_xml_rfrom').val();
            var rto = $(this).parent('div.connector_lei_see_xml').find('input.connector_lei_see_xml_rto').val();
            
            $(this).attr('href', $(this).data('baseUrl') + "&rfrom=" + rfrom + "&rto=" + rto);
        });        
        
    });
})(jQuery);