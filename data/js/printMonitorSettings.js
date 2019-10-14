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

/*
$('document').ready(function() {
    $('.edit_category').click(function() {
        $tr = $(this).closest('tr');
        alert("You want to edit: Category with ID " + $('.category-id', $tr).text() + " & Name: " + $('.category-name', $tr).text());        
    });
});
*/

window.addEventListener("load", init, false);