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

window.addEventListener("load", init, false);