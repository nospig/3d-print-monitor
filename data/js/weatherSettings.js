function init()
{   
    if ($('#weatherEnabled').is(":checked"))
    {
        $("#weatherForm").show();
    }
    else
    {
        $("#weatherForm").hide();
    }

    $("#weatherEnabled").click(function()
    {
        if($(this).is(":checked"))
        {
            $("#weatherForm").show();
        }
        else
        {
            $("#weatherForm").hide();
        }
    });
}

window.addEventListener("load", init, false);