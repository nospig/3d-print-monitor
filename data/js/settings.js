function init()
{
    if ($('#displayCycleMode').is(":checked")) {
        $("#chooseDisplayForm").hide();
    }
    else {
        $("#chooseDisplayForm").show();
    }

    $("#displayCycleMode").click(function () {
        if ($(this).is(":checked")) {
            $("#chooseDisplayForm").hide();
        }
        else {
            $("#chooseDisplayForm").find('input:radio:nth(0)').prop('checked', true);
            $("#chooseDisplayForm").show();
        }
    });
}

$(function () {
    $('.confirmResetSettings').click(function (e) {
        var options = {
            messageHeader: 'Reset Settings?',
            modalVerticalCenter: true,
            backgroundBlur: ['#header_wrap, #main_content_wrap, #footer_wrap'],
            autoFocusOnConfirmBtn: false,
          };

        e.preventDefault();
        $.confirmModal('Are you sure you want to reset?', options, function (el) {
            window.location.href = "/resetSettings.html";
        });
    });

    $('.confirmForgetWifi').click(function (e) {
        var options = {
            messageHeader: 'Forget WiFi?',
            modalVerticalCenter: true,
            backgroundBlur: ['#header_wrap, #main_content_wrap, #footer_wrap'],
            autoFocusOnConfirmBtn: false,
          };

        e.preventDefault();
        $.confirmModal('Are you sure you want to forget the current Wifi settings?', options, function (el) {
            window.location.href = "/forgetWiFi.html";
        });
    });
});

window.addEventListener("load", init, false);