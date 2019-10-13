#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "SettingsManager.h"

const int CURRENT_WEATHER_INTERVAL      = 10 * MINUTES_MULT;
const int PRINT_MONITOR_INERVAL         = 30 * SECONDS_MULT;

void SettingsManager::init()
{
    // for testing now as settings not saved to start with
    //SPIFFS.remove(SETTINGS_FILE_NAME);

    if(!SPIFFS.exists(SETTINGS_FILE_NAME))
    {
        //Serial.println("No settings found, creating.");
        resetSettings();
        saveSettings();
    }
    
    loadSettings();

    settingsChanged = false;
}

void SettingsManager::resetSettings()
{
    data.openWeatherMapAPIKey = "";
    data.openWeatherLocationID = "";

    data.dispayMode = DisplayMode_Weather;
    data.displayBrightness = 100;
    data.displayMetric = true;

    data.currentWeatherInterval = CURRENT_WEATHER_INTERVAL;
    data.printMonitorInterval = PRINT_MONITOR_INERVAL;

    data.octoPrintAddress = "";
    data.octoPrintPort = 80;
    data.octoPrintUsername = "";
    data.octoPrintPassword = "";
    data.octoPrintAPIKey = "";
    data.octoPrintMonitorEnabled = false;
    data.octoPrintDisplayName = "";

    data.utcOffsetSeconds = 0;

    saveSettings();
    settingsChanged = true;
}

void SettingsManager::loadSettings()
{
    File jsonSettings;
    DynamicJsonDocument doc(SETTINGS_JSON_SIZE);

    jsonSettings = SPIFFS.open(SETTINGS_FILE_NAME, "r");
    deserializeJson(doc, jsonSettings);

    data.openWeatherMapAPIKey = (const char*)doc["WeatherAPIKey"];
    data.openWeatherLocationID = (const char*)doc["WeatherLLocationID"];

    int mode = doc["DisplayMode"];
    data.dispayMode = (DisplayMode)mode;
    data.displayBrightness = doc["DisplayBrightness"];
    data.displayMetric = doc["DisplayMetric"];

    data.currentWeatherInterval = doc["CurrentWeatherInterval"];
    data.printMonitorInterval = doc["PrinterMonitorInterval"];

    data.octoPrintAddress = (const char*)doc["OctoPrintAddress"];
    data.octoPrintPort = doc["OctoPrintPort"];
    data.octoPrintUsername = (const char*)doc["OctoPrintUsername"];
    data.octoPrintPassword = (const char*)doc["OctoPrintPassword"];
    data.octoPrintAPIKey = (const char*)doc["OctoPrintAPIKey"];
    data.octoPrintDisplayName = (const char*)doc["OctoPrintDisplayName"];
    data.octoPrintMonitorEnabled = doc["OctoPrintEnabled"];

    data.utcOffsetSeconds = doc["utcOffset"];

    jsonSettings.close();
    
    // testing
    //Serial.println();
    //serializeJson(doc, Serial);
    //Serial.println();
}

void SettingsManager::saveSettings()
{
    File jsonSettings;
    DynamicJsonDocument doc(SETTINGS_JSON_SIZE);   

    doc["WeatherAPIKey"] = data.openWeatherMapAPIKey;
    doc["WeatherLLocationID"] = data.openWeatherLocationID;
    doc["DisplayMode"] = (int)data.dispayMode;
    doc["DisplayBrightness"] = data.displayBrightness;
    doc["DisplayMetric"] = data.displayMetric;
    doc["CurrentWeatherInterval"] = data.currentWeatherInterval;
    doc["PrinterMonitorInterval"] = data.printMonitorInterval;
    doc["utcOffset"] = data.utcOffsetSeconds;

    doc["OctoPrintAddress"] = data.octoPrintAddress;
    doc["OctoPrintPort"] = data.octoPrintPort;
    doc["OctoPrintUsername"] = data.octoPrintUsername;
    doc["OctoPrintPassword"] = data.octoPrintPassword;
    doc["OctoPrintAPIKey"] = data.octoPrintAPIKey;
    doc["OctoPrintDisplayName"] = data.octoPrintDisplayName;
    doc["OctoPrintEnabled"] = data.octoPrintMonitorEnabled;

    jsonSettings = SPIFFS.open(SETTINGS_FILE_NAME, "w");
    if(jsonSettings)
    {
        serializeJson(doc, jsonSettings);
        //Serial.println("Settings file saved.");
        jsonSettings.close();
    }
    else
    {
        Serial.println("Unable to save settings file.");
    }
    
    // testing
    //serializeJson(doc, Serial); 
    //Serial.println();
}

void SettingsManager::updateSettings()
{
    saveSettings();
    settingsChanged = true;
}

String SettingsManager::getOpenWeatherApiKey()
{
    return data.openWeatherMapAPIKey;
}

void SettingsManager::setOpenWeatherApiKey(String apiKey)
{
    if(apiKey != data.openWeatherMapAPIKey)
    {
        data.openWeatherMapAPIKey = apiKey;
        updateSettings();
    }
}

String SettingsManager::getOpenWeatherlocationID()
{
    return data.openWeatherLocationID;
}

void SettingsManager::setOpenWeatherlocationID(String locationID)
{
    if(locationID != data.openWeatherLocationID)
    {
        data.openWeatherLocationID = locationID;
        updateSettings();
    }
}

DisplayMode SettingsManager::getDisplayMode()
{
    return data.dispayMode;
}

void SettingsManager::setDisplayMode(DisplayMode displayMode)
{
    if(displayMode != data.dispayMode)
    {
        data.dispayMode = displayMode;
        updateSettings();
    }
}

bool SettingsManager::getDisplayMetric()
{
    return data.displayMetric;
}

void SettingsManager::setDisplayMetric(bool metric)
{
    if(metric != data.displayMetric)
    {
        data.displayMetric = metric;
        updateSettings();
    }
}

int SettingsManager::getCurrentWeatherInterval()
{
    return data.currentWeatherInterval;
}

void SettingsManager::setCurrentWeatherInterval(int interval)
{
    if(data.currentWeatherInterval != interval)
    {
        data.currentWeatherInterval = interval;
        updateSettings();
    }
}

int SettingsManager::getPrintMonitorInterval()
{
    return data.printMonitorInterval;
}

void SettingsManager::setPrintMonitorInterval(int interval)
{
    if(data.printMonitorInterval != interval)
    {
        data.printMonitorInterval = interval;
        updateSettings();
    }
}

String SettingsManager::getOctoPrintAddress()
{
    return data.octoPrintAddress;
}

void SettingsManager::setOctoPrintAddress(String address)
{
    if(data.octoPrintAddress != address)
    {
        data.octoPrintAddress = address;
        updateSettings();
    }
}

int SettingsManager::getOctoPrintPort()
{
    return data.octoPrintPort;
}

void SettingsManager::setOctoPrintPort(int port)
{
    if(data.octoPrintPort != port)
    {
        data.octoPrintPort = port;
        updateSettings();
    }
}

String SettingsManager::getOctoPrintUsername()
{
    return data.octoPrintUsername;
}

void SettingsManager::setOctoPrintUsername(String userName)
{
    if(data.octoPrintUsername != userName)
    {
        data.octoPrintUsername = userName;
        updateSettings();
    }
}

String SettingsManager::getOctoPrintPassword()
{
    return data.octoPrintPassword;
}

void SettingsManager::setOctoPrintPassword(String password)
{
    if(data.octoPrintPassword != password)
    {
        data.octoPrintPassword = password;
        updateSettings();
    }
}

String SettingsManager::getOctoPrintAPIKey()
{
    return data.octoPrintAPIKey;
}

void SettingsManager::setOctoPrintAPIKey(String apiKey)
{
    if(data.octoPrintAPIKey != apiKey)
    {
        data.octoPrintAPIKey = apiKey;
        updateSettings();
    }
}

String SettingsManager::getOctoPrintDisplayName()
{
    return data.octoPrintDisplayName;
}

void SettingsManager::setOctoPrintDisplayName(String displayName)
{
    if(data.octoPrintDisplayName != displayName)
    {
        data.octoPrintDisplayName = displayName;
        updateSettings();
    }
}

bool SettingsManager::getOctoPrintEnabled()
{
    return data.octoPrintMonitorEnabled;
}

void SettingsManager::setOctoPrintEnabled(bool enabled)
{
    if(data.octoPrintMonitorEnabled != enabled)
    {
        data.octoPrintMonitorEnabled = enabled;
        updateSettings();
    }
}

long SettingsManager::getUtcOffset()
{
    return data.utcOffsetSeconds;
}

void SettingsManager::setUtcOffset(long utcOffset)
{
    if(data.utcOffsetSeconds != utcOffset)
    {
        data.utcOffsetSeconds = utcOffset;
        updateSettings();
    }
}

int SettingsManager::getDisplayBrightness()
{
    return data.displayBrightness;
}

void SettingsManager::setDisplayBrightness(int brightnessPercent)
{
    if(brightnessPercent < 0)
    {
        brightnessPercent = 0;
    }
    if(brightnessPercent > 100)
    {
        brightnessPercent = 100;
    }
     if(data.displayBrightness != brightnessPercent)
    {
        data.displayBrightness = brightnessPercent;
        updateSettings();
    }
}

bool SettingsManager::getSettingsChanged()
{
    return settingsChanged;
}

void SettingsManager::resetSettingsChanged()
{
    settingsChanged = false;
}

