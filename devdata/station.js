var websocket;

function init()
{   
    openWebSocket();
}

function updateInternetCurrent(messageData)
{
    var current = messageData.currentReadings;
    var currentValue;
    var observedTime;
   
    $("#internetWeatherList").empty();

    if(current.metric)
    {
        currentValue = current.temp.toFixed(1) + "C - " + current.description;
    }
    else
    {
        currentValue = current.temp.toFixed(1) + "F - " + current.description;
    }
    $("#internetWeatherList").append('<li class="list-group-item">' + currentValue + '</li>');
    
    currentValue = "Humidity: " + current.humidity + "%";
    $("#internetWeatherList").append('<li class="list-group-item">' + currentValue + '</li>');
    
    currentValue = "Wind speed: " + current.windSpeed + "m/s from " + current.windDirection + "&#176";
    $("#internetWeatherList").append('<li class="list-group-item">' + currentValue + '</li>');

    observedTime = new Date(current.time * 1000);
    currentValue = "Observed at: " + observedTime.toLocaleTimeString("en-GB") + " on " + observedTime.toLocaleDateString("en-GB");
    $("#internetWeatherList").append('<li class="list-group-item">' + currentValue + '</li>');
}

function updateMonitorInfo(messageData)
{
    $("#monitorInfoList").empty();

    if(messageData.enabled)
    {
        if(messageData.validPrintData)
        {
            $("#monitorInfoList").append('<li class="list-group-item">' + 'Printer state: ' + messageData.printState + '</li>');    
        }
        else
        {
            $("#monitorInfoList").append('<li class="list-group-item">' + 'No valid data received.' + '</li>');    
        }
    }
    else
    {
        $("#monitorInfoList").append('<li class="list-group-item">' + 'Monitor not enabled.' + '</li>');
    }
}

function openWebSocket() 
{
    websocket = new WebSocket('ws://' + document.location.host + '/ws');
    websocket.onopen = function(evt) { onOpen(evt) };
    websocket.onclose = function(evt) { onClose(evt) };
    websocket.onmessage = function(evt) { onMessage(evt) };
    websocket.onerror = function(evt) { onError(evt) };
}

function onOpen(evt)
{    
    $("#dashboardTitle").html("Dashboard - connected");
}

function onClose(evt)
{
    $("#dashboardTitle").html("Dashboard - not connected");
}

function onMessage(evt)
{
    //console.log(evt.data);

    var messageData = JSON.parse(evt.data);

    switch(messageData.type)
    {
        case "currentWeather":
            updateInternetCurrent(messageData);
            break;
        case "monitorInfo":
            updateMonitorInfo(messageData);
            break;            
    }
}

function onError(evt)
{
}

function doSend(message)
{
    websocket.send(message);
}

window.addEventListener("load", init, false);
