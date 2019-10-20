#include <Arduino.h>
#include <ArduinoOTA.h>
#include <TaskScheduler.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESPAsyncWifiManager.h>
#include "OpenWeatherMapCurrent.h"
#include "SettingsManager.h"
#include "Settings.h"
#include "PrintMonitor.h"
#include "DisplayTFT.h"
#include "WebServer.h"
#include "OctoPrintMonitor.h"
#include <FS.h>

// globals
Scheduler taskScheduler;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
WebServer webServer;
DisplayBase* display;
OpenWeatherMapCurrent currentWeatherClient;
SettingsManager settingsManager;
DNSServer dns;
OctoPrintMonitor octoPrintMonitor;
int currentPrinter;

// tasks
Task connectWifi(0, TASK_ONCE, &connectWifiCallback);
Task getTime(TIME_FETCH_INTERVAL, TASK_FOREVER, &getTimeCallback);
Task getCurrentWeather(60*SECONDS_MULT, TASK_FOREVER, &getCurrentWeatherCallback);
Task updateWiFiStrength(WIFI_STRENGTH_INTERVAL, TASK_FOREVER, &updateWifiStrengthCallback);
Task checkScreenGrabRequested(SCREENGRAB_INTERVAL, TASK_FOREVER, &checkScreenGrabCallback);
Task octoPrintUpdate(5*MINUTES_MULT, TASK_FOREVER, &updatePrinterMonitorCallback);
Task cycleDisplay(30*SECONDS_MULT, TASK_FOREVER, &cycleDisplayCallback);     

// task callbacks

// time

void getTimeCallback()
{
    timeClient.update();
    display->drawCurrentTime(timeClient.getEpochTime(), settingsManager.getClockFormat(), settingsManager.getDateFormat());
}

// weather

void getCurrentWeatherCallback()
{
    if(settingsManager.getWeatherEnabled())
    {
        currentWeatherClient.updateById(settingsManager.getOpenWeatherApiKey(), settingsManager.getOpenWeatherlocationID());
        webServer.updateCurrentWeather(currentWeatherClient.getCurrentData());
    }
    display->drawCurrentWeather(currentWeatherClient.getCurrentData(), settingsManager.getWeatherEnabled());
}

// Printer monitor

void updatePrinterMonitorCallback()
{
    OctoPrinterData* printerData = settingsManager.getPrinterData(currentPrinter);

    if(printerData->enabled)
    {
        octoPrintMonitor.setCurrentPrinter(printerData->address, printerData->port, printerData->apiKey, printerData->username, printerData->password);
        octoPrintMonitor.update();
    }
    
    display->drawOctoPrintStatus(octoPrintMonitor.getCurrentData(), printerData->displayName, printerData->enabled);
    webServer.updatePrintMonitorInfo(octoPrintMonitor.getCurrentData(), printerData->displayName, printerData->enabled);

    Serial.println("updatePrinterMonitorCallback");
}

// wifi

void connectWifiCallback()
{
    AsyncWiFiManager wifiManager(webServer.getServer(), &dns);

    settingsManager.init();

    display->setDisplayBrightness(settingsManager.getDisplayBrightness());
    display->setDisplayMetric(settingsManager.getDisplayMetric());
    display->drawStartupDisplay();

    wifiManager.autoConnect("3D Print Monitor");

    setupOtaUpdates();

    Serial.println(WiFi.localIP());

    webServer.init(&settingsManager);

    timeClient.setTimeOffset(settingsManager.getUtcOffset());

    currentWeatherClient.setMetric(settingsManager.getDisplayMetric());
    settingsManager.setSettingsChangedCallback(settingsChangedCallback);
    settingsManager.setPrinterDeletedCallback(printerDeletedCallback);
    delay(WIFI_CONNECTING_DELAY);

    taskScheduler.addTask(getTime);
    taskScheduler.addTask(getCurrentWeather);
    taskScheduler.addTask(updateWiFiStrength);
    taskScheduler.addTask(checkScreenGrabRequested);
    taskScheduler.addTask(octoPrintUpdate);
    taskScheduler.addTask(cycleDisplay);

    // timings
    getCurrentWeather.setInterval(settingsManager.getCurrentWeatherInterval());
    octoPrintUpdate.setInterval(settingsManager.getPrintMonitorInterval());
    cycleDisplay.setInterval(settingsManager.getDisplayCycleInterval());

    getTime.enable();
    getCurrentWeather.enable();     // TODO
    updateWiFiStrength.enable();
    checkScreenGrabRequested.enable();
    cycleDisplay.disable();

    setupDisplay();
    display->setDisplayBrightness(settingsManager.getDisplayBrightness());
    display->clearDisplay();
    display->drawIPAddress(WiFi.localIP().toString());
}

void updateWifiStrengthCallback()
{
    long wifiStrength = WiFi.RSSI();
    display->drawWiFiStrength(wifiStrength);
}

// settings

void settingsChangedCallback()
{
    timeClient.setTimeOffset(settingsManager.getUtcOffset());
    currentWeatherClient.setMetric(settingsManager.getDisplayMetric());

    // best just to force a display clear when changing settings
    display->setDisplayBrightness(settingsManager.getDisplayBrightness());
    display->setDisplayMetric(settingsManager.getDisplayMetric());
    display->clearDisplay();
    setupDisplay();

    getCurrentWeather.setInterval(settingsManager.getCurrentWeatherInterval());
    octoPrintUpdate.setInterval(settingsManager.getPrintMonitorInterval());
    cycleDisplay.setInterval(settingsManager.getDisplayCycleInterval());

    getTime.forceNextIteration();
    getCurrentWeather.forceNextIteration();
    updateWiFiStrength.forceNextIteration();
    octoPrintUpdate.forceNextIteration();
}

void printerDeletedCallback()
{
    settingsManager.setSettingsChangedCallback(nullptr);

    settingsManager.setCurrentDisplay(WEATHER_DISPLAY_SETTING);

    settingsManager.setSettingsChangedCallback(settingsChangedCallback);
}

// screen grabs

void checkScreenGrabCallback()
{
    if(webServer.screenGrabRequested())
    {
        display->serveScreenShot();
        webServer.clearScreenGrabRequest();
    }
}

// display handling

void setupDisplay()
{
    if(settingsManager.getWeatherEnabled() == false && settingsManager.getNumPrinters() == 0)
    {
        return; // TODO
    }

    switch(settingsManager.getCurrentDisplay())
    {
        case CYCLE_DISPLAY_SETTING:
            display->setDisplayMode(DisplayMode_Weather);
            currentPrinter = -1;
            octoPrintUpdate.disable();
            cycleDisplay.enableDelayed(10*SECONDS_MULT);    // TODO
            break;

        case WEATHER_DISPLAY_SETTING:
            display->setDisplayMode(DisplayMode_Weather);
            currentPrinter = -1;
            octoPrintUpdate.disable();
            cycleDisplay.disable();
            break;

        default:
            int printerId = settingsManager.getCurrentDisplay() - 1;
            currentPrinter = printerId;
            octoPrintUpdate.enableIfNot();
            cycleDisplay.disable();

            display->setDisplayMode(DisplayMode_PrintMonitor);
            break;
    }
}

// display cycling

void cycleDisplayCallback()
{
    int nextPrinter;

    nextPrinter = getNextPrinter(currentPrinter);
    if(nextPrinter != -1)
    {
        currentPrinter = nextPrinter;
        octoPrintUpdate.enableIfNot();
        octoPrintUpdate.forceNextIteration();
        display->setDisplayMode(DisplayMode_PrintMonitor);
    }
    else
    {
        // back to weather
        if(currentPrinter != -1)
        {
            display->setDisplayMode(DisplayMode_Weather);
            currentPrinter = nextPrinter;
            octoPrintUpdate.disable();
            display->drawCurrentWeather(currentWeatherClient.getCurrentData(), settingsManager.getWeatherEnabled());
        }
    }
}

int getNextPrinter(int currentPrinter)
{
    int startingPrinter;
    int foundPrinter = -1;

    // -1 means find first printer
    if(currentPrinter == -1)
    {
        startingPrinter = 0;
    }
    else
    {
        startingPrinter = currentPrinter + 1;
    }
    
    for(int i=startingPrinter; i<settingsManager.getNumPrinters() && foundPrinter == -1; i++)
    {
        OctoPrinterData* printerData = settingsManager.getPrinterData(i);
        if(printerData->enabled)
        {
            foundPrinter = i;
        }
    }

    return foundPrinter;
}

// basic setup and loop

void setup() 
{
    Serial.begin(115200);

    SPIFFS.begin();

    display = new DisplayTFT();

    currentWeatherClient.setLanguage("en");

    WiFi.hostname("OctoPrint-Monitor");

    taskScheduler.startNow(); 
    taskScheduler.addTask(connectWifi);
    connectWifi.enable();
}

void loop() 
{
    taskScheduler.execute();
    ArduinoOTA.handle();
}

// OTA, not through webserver for now

void setupOtaUpdates()
{
    // set MD5 hash password here
    // add a file to the project called uploadPassword.ini
    // make it's contents
    // [password]
    // upload_password = password
    // where password is plain text
    ArduinoOTA.setPasswordHash(OTA_MD5_PASSWORD_HASH);

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            Serial.println("Start updating firmware.");
        }
        else
        {
            SPIFFS.end();
            Serial.println("Start updating file system.");
        }
    });

    ArduinoOTA.onEnd([]() 
    {
        Serial.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
    });

    ArduinoOTA.begin();
}