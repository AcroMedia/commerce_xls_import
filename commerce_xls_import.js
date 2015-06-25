/**
 * @file commerce_xls_import.js

 *  Custom JavaScript for the Commerce xls import.
 */

var timer;
(function ($) {
  $(document).ready(function(){
    $('#edit-import-button').click(function(){
      $('#import_status').val(1);
    });

        $('#edit-import-stop').click(function() {
          $('#import_status').val(0);
          clearTimeout(timer);
          //@TODO - is the below comment like a todo or explaining something missing or what?
          // Some code to update the message.
        });

        if ($('input[name="import_status"]').val() == 1) {
          // Start counting after 2 seconds;
          timer = setInterval(function(){start_updates()}, 2000);
        }
  });
  function start_updates() {
    periodic_update_import_status();
  }
  function periodic_update_import_status() {
    $.ajax({
      url: Drupal.settings.basePath + "?q=admin/commerce/products/import_commerce/get-import-status",
      type: "POST",
      dataType: "json",
      data: {},
      success: function(data){
        var message_box = $('#import_status_messages');
        message_box.css('display', 'block');
        message_box.html(data.message);
        if (data.status == 1) {
          location.reload();
        }
      }

    });
  }
})(jQuery);
