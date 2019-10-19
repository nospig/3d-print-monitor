function init()
{   
    if ($('#weatherEnabled').is(":checked"))
    {
        showWeatherForm();
    }
    else
    {
        hideWeatherForm();
    }

    $("#weatherEnabled").click(function()
    {
        if($(this).is(":checked"))
        {
            showWeatherForm();
        }
        else
        {
            hideWeatherForm();
        }
    });
}

function showWeatherForm()
{
    $("#weatherForm").show();
    $("#locationID").prop('required', true);
    $("#weatherApiKey").prop('required', true);
}

function hideWeatherForm()
{
    $("#weatherForm").hide();
    $("#locationID").prop('required', false);
    $("#weatherApiKey").prop('required', false);    
}

window.addEventListener("load", init, false);