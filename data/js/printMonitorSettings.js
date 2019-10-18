function init()
{   
    if ($('#printMonitorEnabled').is(":checked"))
    {
        $("#printMonitorForm").show();
    }
    else
    {
        $("#printMonitorForm").hide();
    }

    $("#printMonitorEnabled").click(function()
    {
        if($(this).is(":checked"))
        {
            $("#printMonitorForm").show();
        }
        else
        {
            $("#printMonitorForm").hide();
        }
    });
}

$(function () {
    $('.confirmDeletePrinter').click(function (e) {
        var options = {
            messageHeader: 'Delete?',
            modalVerticalCenter: true,
            backgroundBlur: ['#header_wrap, #main_content_wrap, #footer_wrap'],
            autoFocusOnConfirmBtn: false,
          };

        e.preventDefault();
        $tr = $(this).closest('tr');
        $.confirmModal('Are you sure you want to delete this printer: ' + $('.display-name', $tr).text() + '?' , options, function (el) {
            window.location.href = "/deletePrinter.html?printerId=" + $('.printer-id', $tr).text();
        });
    });
});

$(".btn[data-target='#editPrinterModal']").click(function() 
{
    var columnValues = $(this).parent().siblings().map(function() 
    {
        return $(this).text();
    }).get();
        
    $.get('getPrinter.html', 'printerId=' + columnValues[0],
        function(response)
        {
            console.log(response);

            $('#editPrinterModal').find('#printerId').val(columnValues[0]);
            $('#editPrinterModal').find('#editDisplayName').val(response.displayName);
            $('#editPrinterModal').find('#editPrintUrl').val(response.address);
            $('#editPrinterModal').find('#editPort').val(response.port);
            $('#editPrinterModal').find('#editUsername').val(response.username);
            $('#editPrinterModal').find('#editPassword').val(response.password);
            $('#editPrinterModal').find('#editAPIKey').val(response.apiKey);
            $('#editPrinterModal').find('#editEnabled').prop('checked', response.enabled);            
        }, "json"
    );
});

window.addEventListener("load", init, false);