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

// tasks
Task connectWifi(0, TASK_ONCE, &connectWifiCallback);
Task getTime(TIME_FETCH_INTERVAL, TASK_FOREVER, &getTimeCallback);
Task getCurrentWeather(60*SECONDS_MULT, TASK_FOREVER, &getCurrentWeatherCallback);
Task updateWiFiStrength(WIFI_STRENGTH_INTERVAL, TASK_FOREVER, &updateWifiStrengthCallback);
Task checkSettingsChanged(SETTINGS_CHANGED_INTERVAL, TASK_FOREVER, &checkSettingsChangedCallback);
Task checkScreenGrabRequested(SCREENGRAB_INTERVAL, TASK_FOREVER, &checkScreenGrabCallback);
Task octoPrintUpdate(5*MINUTES_MULT, TASK_FOREVER, &updatePrinterMonitorCallback);

// task callbacks

// time

void getTimeCallback()
{
    timeClient.update();
    display->drawCurrentTime(timeClient.getEpochTime());
}

// weather

void getCurrentWeatherCallback()
{
    if(settingsManager.getOpenWeatherApiKey() != "" && settingsManager.getOpenWeatherlocationID() != "")
    {
        currentWeatherClient.updateById(settingsManager.getOpenWeatherApiKey(), settingsManager.getOpenWeatherlocationID());
        display->drawCurrentWeather(currentWeatherClient.getCurrentData());
        webServer.updateCurrentWeather(currentWeatherClient.getCurrentData());
    }
}

// Printer monitor

void updatePrinterMonitorCallback()
{
    if(settingsManager.getOctoPrintEnabled())
    {
        octoPrintMonitor.update();
    }
    
    display->drawOctoPrintStatus(octoPrintMonitor.getCurrentData(), settingsManager.getOctoPrintDisplayName(), settingsManager.getOctoPrintEnabled());
    webServer.updatePrintMonitorInfo(octoPrintMonitor.getCurrentData(), settingsManager.getOctoPrintEnabled());
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

    octoPrintMonitor.init(settingsManager.getOctoPrintAddress(), settingsManager.getOctoPrintPort(), 
                          settingsManager.getOctoPrintAPIKey(), settingsManager.getOctoPrintUsername(), settingsManager.getOctoPrintPassword());
    timeClient.setTimeOffset(settingsManager.getUtcOffset());

    currentWeatherClient.setMetric(settingsManager.getDisplayMetric());

    delay(WIFI_CONNECTING_DELAY);
    display->setDisplayMode(settingsManager.getDisplayMode());
    display->setDisplayBrightness(settingsManager.getDisplayBrightness());
    display->startMainDisplay();
    display->drawIPAddress(WiFi.localIP().toString());

    taskScheduler.addTask(getTime);
    taskScheduler.addTask(getCurrentWeather);
    taskScheduler.addTask(updateWiFiStrength);
    taskScheduler.addTask(checkSettingsChanged);
    taskScheduler.addTask(checkScreenGrabRequested);
    taskScheduler.addTask(octoPrintUpdate);

    // timings
    getCurrentWeather.setInterval(settingsManager.getCurrentWeatherInterval());
    octoPrintUpdate.setInterval(settingsManager.getPrintMonitorInterval());

    getTime.enable();
    getCurrentWeather.enable();
    updateWiFiStrength.enable();
    checkSettingsChanged.enable();
    checkScreenGrabRequested.enable();
    octoPrintUpdate.enable();
}

void updateWifiStrengthCallback()
{
    long wifiStrength = WiFi.RSSI();
    display->drawWiFiStrength(wifiStrength);
}

// settings

void checkSettingsChangedCallback()
{
    if(settingsManager.getSettingsChanged())
    {
        settingsManager.resetSettingsChanged();

        timeClient.setTimeOffset(settingsManager.getUtcOffset());

        currentWeatherClient.setMetric(settingsManager.getDisplayMetric());

        octoPrintMonitor.updateSettings(settingsManager.getOctoPrintAddress(), settingsManager.getOctoPrintPort(), 
                                        settingsManager.getOctoPrintAPIKey(), settingsManager.getOctoPrintUsername(), settingsManager.getOctoPrintPassword());

        // best just to force a display clear when changing settings
        display->setDisplayMode(settingsManager.getDisplayMode());
        display->setDisplayBrightness(settingsManager.getDisplayBrightness());
        display->setDisplayMetric(settingsManager.getDisplayMetric());
        display->restartMainDisplay();

        getCurrentWeather.setInterval(settingsManager.getCurrentWeatherInterval());
        octoPrintUpdate.setInterval(settingsManager.getPrintMonitorInterval());

        getTime.forceNextIteration();
        getCurrentWeather.forceNextIteration();
        updateWiFiStrength.forceNextIteration();
        octoPrintUpdate.forceNextIteration();
    }
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


// basic setup and loop

void setup() 
{
    Serial.begin(115200);

    SPIFFS.begin();

    display = new DisplayTFT();

    currentWeatherClient.setLanguage("en");

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