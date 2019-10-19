#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWiFiManager.h>
#include "WebServer.h"
#include "Serverpages/AllPages.h"

// globals
AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");
AsyncEventSource events("/events");

String WebServer::currentWeatherJson = "";
String WebServer::currentPrinterJson = "";
bool WebServer::screenGrabRequest = false;

SettingsManager* WebServer::settingsManager;    

static const char NAV_BAR[] PROGMEM = 
    "<nav class='navbar navbar-expand-sm bg-dark navbar-dark fixed-top'>"
    "<a class='navbar-brand' href='index.html'>OctoPrint Monitor</a>"
    "<ul class='navbar-nav'>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='settings.html'>Settings</a>"
    "</li>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='weatherSettings.html'>Weather</a>"
    "</li>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='printMonitorSettings.html'>Printers</a>"
    "</li>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='screenGrab.html'>Screengrab</a>"
    "</nav>";

// methods

void WebServer::init(SettingsManager* settingsManager)
{
    this->settingsManager = settingsManager;
    
    webSocket.onEvent(onEvent);
    server.addHandler(&webSocket);
    server.addHandler(&events);

// SPIFFS for reference
//request->send(SPIFFS, "/printMonitorSettings.html", String(), false, tokenProcessor);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", index_html, tokenProcessor);
    });

    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", index_html, tokenProcessor);
    });

    server.on("/screenGrab.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", screenGrab_html, tokenProcessor);
    });

    server.on("/settings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        //request->send_P(200, "text/html", settings_html, tokenProcessor);
        request->send(SPIFFS, "/settings.html", String(), false, tokenProcessor);
    });

    server.on("/weatherSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        //request->send_P(200, "text/html", weatherSettings_html, tokenProcessor); 
        request->send(SPIFFS, "/weatherSettings.html", String(), false, tokenProcessor);
    });

    server.on("/printMonitorSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        //request->send_P(200, "text/html", printMonitorSettings_html, tokenProcessor);
        request->send(SPIFFS, "/printMonitorSettings.html", String(), false, tokenProcessor);
    });

    server.on("/updateWeatherSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleUpdateWeatherSettings(request);
        request->redirect("/index.html");
    });

    server.on("/updateDisplaySettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleUpdateDisplaySettings(request);
        request->redirect("/index.html");
    });

    server.on("/updateTimings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleUpdateTimings(request);
        request->redirect("/index.html");
    });

    server.on("/updateClockSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleUpdateClockSettings(request);
        request->redirect("/index.html");
    });

    server.on("/addnewPrinter.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleAddNewPrinter(request);
        request->redirect("/printMonitorSettings.html");
    });

    server.on("/deletePrinter.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleDeletePrinter(request);
        request->redirect("/printMonitorSettings.html");
    });

    server.on("/editPrinter.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleEditPrinter(request);
        request->redirect("/printMonitorSettings.html");
    });

    server.on("/getPrinter.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleGetPrinter(request);
        //request->redirect("/printMonitorSettings.html");
    });

    server.on("/resetSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleResetSettings(request);
        request->redirect("/index.html");
    });

    server.on("/forgetWiFi.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->redirect("/index.html");
        handleForgetWiFi(request);
    });

    server.on("/takeScreenGrab.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleScreenGrab(request);
        request->redirect("/screenGrab.html");
    });

    server.on("/js/jquery.confirmModal.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "application/javascript", confirmModal_js);
    });

//    server.on("/js/printMonitorSettings.js", HTTP_GET, [](AsyncWebServerRequest *request)
//    {
//        request->send_P(200, "application/javascript", printMonitorSettings_js);
//    });

//    server.on("/js/weatherSettings.js", HTTP_GET, [](AsyncWebServerRequest *request)
//    {
//        request->send_P(200, "application/javascript", weatherSettings_js);
//    });

//    server.on("/js/settings.js", HTTP_GET, [](AsyncWebServerRequest *request)
//    {
//        request->send_P(200, "application/javascript", settings_js);
//    });
    
    server.on("/js/station.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "application/javascript", station_js);
    });

    server.on("/css/station.css", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/css", station_css);
    });



    server.serveStatic("/js", SPIFFS, "/js/");
    server.serveStatic("/css", SPIFFS, "/css/");

    server.begin();
}

AsyncWebServer* WebServer::getServer()
{
    return &server;
}

void WebServer::updateCurrentWeather(OpenWeatherMapCurrentData* currentWeather)
{
    if(!currentWeather->validData)
    {
        currentWeatherJson = "";
        return;
    }

    String output;

    const size_t capacity = 1024;   // TODO
    DynamicJsonDocument jsonDoc(capacity);

    jsonDoc["type"] = "currentWeather";

    JsonObject weather = jsonDoc.createNestedObject("currentReadings");
    weather["temp"] = currentWeather->temp;
    weather["humidity"] = currentWeather->humidity;
    weather["windSpeed"] = currentWeather->windSpeed;
    weather["windDirection"] = currentWeather->windDeg;
    weather["description"] = currentWeather->description;
    weather["time"] = currentWeather->observationTime;
    weather["metric"] = settingsManager->getDisplayMetric();

    serializeJson(jsonDoc, output);
    currentWeatherJson = output;

    if(webSocket.count() > 0)
    {
        webSocket.textAll(output);
    }
}
    
void WebServer::updatePrintMonitorInfo(OctoPrintMonitorData* printerInfo, bool enabled)
{
    const size_t capacity = 512;  
    DynamicJsonDocument jsonDoc(capacity);
    String output;

    jsonDoc["type"] = "monitorInfo";

    jsonDoc["enabled"] = enabled;
    jsonDoc["validJobData"] = printerInfo->validJobData;
    jsonDoc["validPrintData"] = printerInfo->validPrintData;
    jsonDoc["printState"] = printerInfo->printState;

    serializeJson(jsonDoc, output);
    currentPrinterJson = output;

    if(webSocket.count() > 0)
    {
        webSocket.textAll(output);
    }
}   

void WebServer::updateClientOnConnect()
{
    if(currentWeatherJson.length() != 0)
    {
        webSocket.textAll(currentWeatherJson);
        webSocket.textAll(currentPrinterJson);
    }
}

void WebServer::onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    if(type == WS_EVT_CONNECT)
    {
        updateClientOnConnect();
    }
}

String WebServer::tokenProcessor(const String& token)
{
    if(token == "NAVBAR")
    {
        return FPSTR(NAV_BAR);
    }
    if(token == "WEATHERLOCATIONKEY")    
    {
        return settingsManager->getOpenWeatherlocationID();
    }
    if(token == "WEATHERAPIKEY")    
    {
        return settingsManager->getOpenWeatherApiKey();
    }
    if(token == "DISPLAYMETRIC")
    {
        if(settingsManager->getDisplayMetric())
        {
            return "Checked";
        }
    }
    if(token == "CURRENTWEATHERINTERVAL")    
    {
        return String(settingsManager->getCurrentWeatherInterval() / SECONDS_MULT);
    }
    if(token == "PRINTMONITORINTERVAL")    
    {
        return String(settingsManager->getPrintMonitorInterval() / SECONDS_MULT);
    }
    if(token == "UTCOFFSET")
    {
        return String(settingsManager->getUtcOffset() / 3600.0f);
    }
    if(token == "BRIGHTNESS")
    {
        return String(settingsManager->getDisplayBrightness());
    }
    if(token == "PRINTERTABLE")
    {
       return createPrinterList();
    }
    if(token == "DISPLAYTABLE")
    {
       return createDisplayList();
    }
    if(token == "ADDPRINTERENABLED")
    {
       if(settingsManager->getNumPrinters() >= MAX_PRINTERS)
       {
           return "disabled";
       }
    }
    if(token == "WEATHERENABLED")
    {
        if(settingsManager->getWeatherEnabled() == true)
        {
            return "Checked";
        }
    }    
    if(token == "DISPLAYCYCLEMODE")
    {
        if(settingsManager->getCurrentDisplay() == CYCLE_DISPLAY_SETTING)
        {
            return "Checked";
        }
    }    

    return String();
}

String WebServer::createPrinterList()
{
    int numPrinters = settingsManager->getNumPrinters();
    String response;

    // lots of string use here, bad?
    for(int i=0; i<numPrinters; i++)
    {
        String printerRow;
        char buffer[256];
        OctoPrinterData* data = settingsManager->getPrinterData(i);
        const char deleteButton[] = "<button type='button' class='btn btn-danger mr-2 confirmDeletePrinter'>Delete</button>";
        const char editButton[] = "<button type='button' class='btn btn-primary mr-2' data-toggle='modal' data-target='#editPrinterModal'>Edit</button>";
        const char enabledBox[] = "<td><input type='checkbox' disabled checked></td>";
        const char disabledBox[] = "<td><input type='checkbox' disabled></td>";
        const char *checkbox;

        if(data->enabled)
        {
            checkbox = enabledBox;
        }
        else
        {
            checkbox = disabledBox;
        }
        
        sprintf(buffer, "<tr><td class='printer-id'>%d</td><td class='display-name'>%s</td><td>%s</td><td>%d</td>%s<td>%s%s</td></tr>", 
            i+1, data->displayName.c_str(), data->address.c_str(), data->port, checkbox, editButton, deleteButton);
        printerRow = String(buffer);

        response += printerRow;
    }

    return response;
}

String WebServer::createDisplayList()
{
    int numPrinters = settingsManager->getNumPrinters();
    String response;
    String button;
    int currentDisplay = settingsManager->getCurrentDisplay();
    String checked = "";

    if(currentDisplay == WEATHER_DISPLAY_SETTING)
    {
        checked = "checked";
    }
    response += createDisplayButton(WEATHER_DISPLAY_SETTING, checked, "Current Weather");
    
    for(int i=0; i<numPrinters; i++)
    {
        OctoPrinterData* data = settingsManager->getPrinterData(i);
        String id = "Printer" + i;
        if(currentDisplay == i + 1)
        {
            checked = "checked";
        }
        else
        {
            checked = "";
        }
        
        response += createDisplayButton(i + 1, checked, data->displayName);
    }

    return response;
}

String WebServer::createDisplayButton(int id, String checked, String title)
{
    String button;
    char buffer[256];
    const char buttonHeader[] = "<div class='form-group'><div class='form-check'><label class='form-check-label'>";
    const char buttonInfo[] = "<input type='radio' class='form-check-input' value='%d' name='optdisplay' %s>%s";
    const char buttonFooter[] = "</label></div></div>";
    
    sprintf(buffer, buttonInfo, id, checked.c_str(), title.c_str());
    
    button += buttonHeader;
    button += buffer;
    button += buttonFooter;

    return button;
}

void WebServer::handleUpdateWeatherSettings(AsyncWebServerRequest* request)
{
    if(request->hasParam("openWeatherLocation"))
    {
        AsyncWebParameter* p = request->getParam("openWeatherLocation");
        settingsManager->setOpenWeatherlocationID(p->value());
    }
    if(request->hasParam("openWeatherApiKey"))
    {
        AsyncWebParameter* p = request->getParam("openWeatherApiKey");
        settingsManager->setOpenWeatherApiKey(p->value());
    }
    if(request->hasParam("weatherEnabled"))
    {        
        settingsManager->setWeatherEnabled(true);
    }
    else
    {
        settingsManager->setWeatherEnabled(false);
    }
    if(request->hasParam("displayMetric"))
    {        
        settingsManager->setDisplayMetric(true);
    }
    else
    {
        settingsManager->setDisplayMetric(false);
    }   
}

void WebServer::handleUpdateDisplaySettings(AsyncWebServerRequest* request)
{
    if(request->hasParam("displayCycleMode"))
    {
        settingsManager->setCurrentDisplay(CYCLE_DISPLAY_SETTING);
    }
    else if(request->hasParam("optdisplay"))
    {
        AsyncWebParameter* p = request->getParam("optdisplay");
        
        settingsManager->setCurrentDisplay(p->value().toInt());
    }
    if(request->hasParam("brightness"))
    {
        AsyncWebParameter* p = request->getParam("brightness");
        settingsManager->setDisplayBrightness(p->value().toInt());
    }
}

void WebServer::handleUpdateTimings(AsyncWebServerRequest* request)
{
    if(request->hasParam("currentWeatherInterval"))
    {
        AsyncWebParameter* p = request->getParam("currentWeatherInterval");
        settingsManager->setCurrentWeatherInterval(p->value().toInt() * SECONDS_MULT);
    }
    if(request->hasParam("printMonitorInterval"))
    {
        AsyncWebParameter* p = request->getParam("printMonitorInterval");
        settingsManager->setPrintMonitorInterval(p->value().toInt() * SECONDS_MULT);
    }
    
}

void WebServer::handleUpdateClockSettings(AsyncWebServerRequest* request)
{
    if(request->hasParam("utcOffset"))
    {
        AsyncWebParameter* p = request->getParam("utcOffset");
        settingsManager->setUtcOffset(p->value().toFloat() * 3600.0f);
    }
}

void WebServer::handleAddNewPrinter(AsyncWebServerRequest* request)
{
    bool enabled = false;

    if(request->hasParam("printerEnabled"))
    {
        enabled = true;
    }

    settingsManager->addNewPrinter(
        request->getParam("octoPrintUrl")->value(),
        request->getParam("octoPrintPort")->value().toInt(),
        request->getParam("octoPrintUsername")->value(),
        request->getParam("octoPrintPassword")->value(),
        request->getParam("octoPrintAPIKey")->value(),
        request->getParam("octoPrintDisplayName")->value(),
        enabled
    );
}

void WebServer::handleDeletePrinter(AsyncWebServerRequest* request)
{
    int printerId;

    printerId = request->getParam("printerId")->value().toInt(); 
    printerId--;

    settingsManager->deletePrinter(printerId);
}

void WebServer::handleEditPrinter(AsyncWebServerRequest* request)
{
    int printerId;
    bool enabled = false;

    printerId = request->getParam("printerId")->value().toInt(); 
    printerId--;

    if(request->hasParam("editEnabled"))
    {
        enabled = true;
    }

    settingsManager->editPrinter(
        printerId,
        request->getParam("editPrintUrl")->value(),
        request->getParam("editPort")->value().toInt(),
        request->getParam("editUsername")->value(),
        request->getParam("editPassword")->value(),
        request->getParam("editAPIKey")->value(),
        request->getParam("editDisplayName")->value(),
        enabled
    );
}

void WebServer::handleGetPrinter(AsyncWebServerRequest* request)
{
    AsyncWebParameter* p = request->getParam("printerId");
    int printerID;

    printerID = p->value().toInt();
    printerID--;

    OctoPrinterData* printer = settingsManager->getPrinterData(printerID);

    const size_t capacity = 512;  
    DynamicJsonDocument doc(capacity);
    String reponse;

    doc["address"] = printer->address;
    doc["port"] = printer->port;
    doc["username"] = printer->username;
    doc["password"] = printer->password;
    doc["apiKey"] = printer->apiKey;
    doc["displayName"] = printer->displayName;
    doc["enabled"] = printer->enabled;

    serializeJson(doc, reponse);

    request->send(200, "application/json", reponse);
}

void WebServer::handleForgetWiFi(AsyncWebServerRequest* request)
{
    DNSServer dns;
    AsyncWiFiManager wifiManager(getServer(), &dns);
    wifiManager.resetSettings();
    ESP.restart();
}

void WebServer::handleResetSettings(AsyncWebServerRequest* request)
{
    settingsManager->resetSettings();
}

void WebServer::handleScreenGrab(AsyncWebServerRequest* request)
{
    screenGrabRequest = true;
}

bool WebServer::screenGrabRequested()
{
    return screenGrabRequest;
}

void WebServer::clearScreenGrabRequest()
{
    screenGrabRequest = false;
}
