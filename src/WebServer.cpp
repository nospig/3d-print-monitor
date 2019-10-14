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
    "<a class='nav-link' href='printMonitorSettings.html'>OctoPrint Monitor</a>"
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
        request->send_P(200, "text/html", settings_html, tokenProcessor);
    });

    server.on("/weatherSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/html", weatherSettings_html, tokenProcessor); 
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

    server.on("/updatePrintMonitorSettings.html", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        handleUpdatePrintMonitorSettings(request);
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

    server.on("/js/settings.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "application/javascript", settings_js);
    });
    
    server.on("/css/station.css", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "text/css", station_css);
    });

    server.on("/js/station.js", HTTP_GET, [](AsyncWebServerRequest *request)
    {
        request->send_P(200, "application/javascript", station_js);
    });
    

#define DEBUG_REQUESTS
#ifdef DEBUG_REQUESTS
// debugging
    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.printf("NOT_FOUND: ");
        if (request->method() == HTTP_GET)
            Serial.printf("GET");
        else if (request->method() == HTTP_POST)
            Serial.printf("POST");
        else if (request->method() == HTTP_DELETE)
            Serial.printf("DELETE");
        else if (request->method() == HTTP_PUT)
            Serial.printf("PUT");
        else if (request->method() == HTTP_PATCH)
            Serial.printf("PATCH");
        else if (request->method() == HTTP_HEAD)
            Serial.printf("HEAD");
        else if (request->method() == HTTP_OPTIONS)
            Serial.printf("OPTIONS");
        else
            Serial.printf("UNKNOWN");
        Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if (request->contentLength())
        {
            Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for (i = 0; i < headers; i++)
        {
            AsyncWebHeader *h = request->getHeader(i);
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for (i = 0; i < params; i++)
        {
            AsyncWebParameter *p = request->getParam(i);
            if (p->isFile())
            {
                Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            }
            else if (p->isPost())
            {
                Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
            else
            {
                Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });
#endif // DEBUG_REQUESTS

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
    if(token == "DISPLAY1CHECKED")
    {
        if(settingsManager->getDisplayMode() == DisplayMode_Weather)
        {
            return "Checked";
        }
    }
    if(token == "DISPLAY2CHECKED")
    {
        if(settingsManager->getDisplayMode() == DisplayMode_PrintMonitor)
        {
            return "Checked";
        }
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
    if(token == "PRINTMONITORENABLED")
    {
        if(settingsManager->getOctoPrintEnabled() == true)
        {
            return "Checked";
        }
    }
    if(token == "PRINTERTABLE")
    {
       return createPrinterList();
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
        const char deleteButton[] = "<button type='button' class='btn btn-danger mb-2 mr-2 confirmDeletePrinter'>Delete</button>";
        const char editButton[] = "<button type='button' class='btn btn-primary mb-2 mr-2' data-toggle='modal' data-target='#editPrinterModal'>Edit</button>";

        sprintf(buffer, "<tr><td class='printer-id'>%d</td><td class='display-name'>%s</td><td>%s</td><td>%d</td><td>%s%s</td></tr>", 
            i+1, data->displayName.c_str(), data->address.c_str(), data->port, editButton, deleteButton);
        printerRow = String(buffer);

        response += printerRow;
    }

    return response;
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
    if(request->hasParam("optdisplay"))
    {
        AsyncWebParameter* p = request->getParam("optdisplay");
        
        if(p->value() == "display1")
        {
            settingsManager->setDisplayMode(DisplayMode_Weather);
        }
        if(p->value() == "display2")
        {
            settingsManager->setDisplayMode(DisplayMode_PrintMonitor);
        }
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
    Serial.println("Server got add new printer");

    settingsManager->addNewPrinter(
        request->getParam("octoPrintUrl")->value(),
        request->getParam("octoPrintPort")->value().toInt(),
        request->getParam("octoPrintUsername")->value(),
        request->getParam("octoPrintPassword")->value(),
        request->getParam("octoPrintAPIKey")->value(),
        request->getParam("octoPrintDisplayName")->value()
    );
}

void WebServer::handleDeletePrinter(AsyncWebServerRequest* request)
{
    Serial.println("Server got delete printer");

    AsyncWebParameter* p = request->getParam("printerId");
    Serial.print("Printer ID is: ");
    Serial.println(p->value().toInt());
}

void WebServer::handleEditPrinter(AsyncWebServerRequest* request)
{
    Serial.println("Server got edit printer");

    AsyncWebParameter* p = request->getParam("printerId");
    Serial.print("Printer ID is: ");
    Serial.println(p->value().toInt());

    p = request->getParam("editDisplayName");
    Serial.print("Printer display name is: ");
    Serial.println(p->value());

}

void WebServer::handleGetPrinter(AsyncWebServerRequest* request)
{
    Serial.println("Server got get printer");

    AsyncWebParameter* p = request->getParam("printerId");
    Serial.print("Printer ID is: ");
    Serial.println(p->value());    

const char reponse[] = "{\n"
"    \"address\": \"192.168.1.41\",\n"
"    \"port\": 80,\n"
"    \"username\": \"mgipson\",\n"
"    \"password\": \"fingers10\",\n"
"    \"apiKey\": \"FABD3A784FAA4843B7FE34CB5E14AEE3\",\n"
"    \"displayName\": \"Ender GET TEST\",\n"
"    \"enabled\": true\n"
"}";

    request->send(200, "application/json", reponse);
}

void WebServer::handleUpdatePrintMonitorSettings(AsyncWebServerRequest* request)
{
/*
    if(request->hasParam("printMonitorEnabled"))
    {        
        settingsManager->setOctoPrintEnabled(true);
    }
    else
    {
        settingsManager->setOctoPrintEnabled(false);
    }

    if(request->hasParam("octoPrintUrl"))
    {
        AsyncWebParameter* p = request->getParam("octoPrintUrl");
        settingsManager->setOctoPrintAddress(p->value());
    }
    if(request->hasParam("octoPrintPort"))
    {
        AsyncWebParameter* p = request->getParam("octoPrintPort");
        settingsManager->setOctoPrintPort(p->value().toInt());
    }
    if(request->hasParam("octoPrintUsername"))
    {
        AsyncWebParameter* p = request->getParam("octoPrintUsername");
        settingsManager->setOctoPrintUsername(p->value());
    }
    if(request->hasParam("octoPrintPassword"))
    {
        AsyncWebParameter* p = request->getParam("octoPrintPassword");
        settingsManager->setOctoPrintPassword(p->value());
    }
    if(request->hasParam("octoPrintAPIKey"))
    {
        AsyncWebParameter* p = request->getParam("octoPrintAPIKey");
        settingsManager->setOctoPrintAPIKey(p->value());
    }
    if(request->hasParam("octoPrintDisplayName"))
    {
        AsyncWebParameter* p = request->getParam("octoPrintDisplayName");
        settingsManager->setOctoPrintDisplayName(p->value());
    }   
*/ 
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
