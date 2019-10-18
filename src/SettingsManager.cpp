#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "SettingsManager.h"

const int CURRENT_WEATHER_INTERVAL      = 10 * MINUTES_MULT;
const int PRINT_MONITOR_INERVAL         = 30 * SECONDS_MULT;

// TODO, calculate sizes
const int PRINTER_JSON_SIZE  = 512;           
const int SETTINGS_JSON_SIZE = 768;

void SettingsManager::init()
{
    data.numPrinters = 0;

    for(int i=0; i<MAX_PRINTERS; i++)
    {
        printersData[i] = new OctoPrinterData;
    }

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
    data.weatherEnabled = true;

    data.dispayMode = DisplayMode_Weather;
    data.displayBrightness = 100;
    data.displayMetric = true;

    data.currentWeatherInterval = CURRENT_WEATHER_INTERVAL;
    data.printMonitorInterval = PRINT_MONITOR_INERVAL;

    data.numPrinters = 0;
    data.octoPrintMonitorEnabled = false;

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
    data.weatherEnabled = doc["WeatherEnabled"];

    int mode = doc["DisplayMode"];
    data.dispayMode = (DisplayMode)mode;
    data.displayBrightness = doc["DisplayBrightness"];
    data.displayMetric = doc["DisplayMetric"];

    data.currentWeatherInterval = doc["CurrentWeatherInterval"];
    data.printMonitorInterval = doc["PrinterMonitorInterval"];

    data.octoPrintMonitorEnabled = doc["OctoPrintEnabled"];
    data.numPrinters = doc["PrinterCount"];

    data.utcOffsetSeconds = doc["utcOffset"];

    jsonSettings.close();
    
    // testing
    Serial.println();
    serializeJson(doc, Serial);
    Serial.println();

    loadPrinters();
}

void SettingsManager::loadPrinters()
{
    char buffer[32];

    for(int i=0; i<data.numPrinters; i++)
    {
        File printerSettings;
        DynamicJsonDocument doc(PRINTER_JSON_SIZE);
        sprintf(buffer, "/printer%d.json", i);
        OctoPrinterData* printer = printersData[i];

        printerSettings = SPIFFS.open(buffer, "r");
        deserializeJson(doc, printerSettings);

        printer->address = (const char*)doc["Address"];
        printer->port = doc["Port"];
        printer->username = (const char*)doc["Username"];
        printer->password = (const char*)doc["Password"];
        printer->apiKey = (const char*)doc["APIKey"];
        printer->displayName = (const char*)doc["DisplayName"];
        printer->enabled = doc["Enabled"];

        printerSettings.close();

        // testing
        Serial.println();
        serializeJson(doc, Serial);
        Serial.println();        
    }
}

void SettingsManager::saveSettings()
{
    File jsonSettings;
    DynamicJsonDocument doc(SETTINGS_JSON_SIZE);   

    doc["WeatherAPIKey"] = data.openWeatherMapAPIKey;
    doc["WeatherLLocationID"] = data.openWeatherLocationID;
    doc["WeatherEnabled"] = data.weatherEnabled;
    doc["DisplayMode"] = (int)data.dispayMode;
    doc["DisplayBrightness"] = data.displayBrightness;
    doc["DisplayMetric"] = data.displayMetric;
    doc["CurrentWeatherInterval"] = data.currentWeatherInterval;
    doc["PrinterMonitorInterval"] = data.printMonitorInterval;
    doc["utcOffset"] = data.utcOffsetSeconds;

    doc["OctoPrintEnabled"] = data.octoPrintMonitorEnabled;
    doc["PrinterCount"] = data.numPrinters;

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
    serializeJson(doc, Serial); 
    Serial.println();

    savePrinters();
}

void SettingsManager::savePrinters()
{
    char buffer[32];

    for(int i=0; i<data.numPrinters; i++)
    {
        File printerSettings;
        DynamicJsonDocument doc(PRINTER_JSON_SIZE);
        sprintf(buffer, "/printer%d.json", i);
        OctoPrinterData* printer = printersData[i];

        doc["Address"] = printer->address;
        doc["Port"] = printer->port;
        doc["Username"] = printer->username;
        doc["Password"] = printer->password;
        doc["APIKey"] = printer->apiKey;
        doc["DisplayName"] = printer->displayName;
        doc["Enabled"] = printer->enabled;

        printerSettings = SPIFFS.open(buffer, "w");
        if(printerSettings)
        {
            serializeJson(doc, printerSettings);
            //Serial.println("Printer file saved.");
            printerSettings.close();

            // testing
            serializeJson(doc, Serial); 
            Serial.println();
        }
        else
        {
            Serial.println("Unable to save printer.");
        }
    }
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

void SettingsManager::setWeatherEnabled(bool enabled)
{
    if(enabled != data.weatherEnabled)
    {
        data.weatherEnabled = enabled;
        updateSettings();
    }
}

bool SettingsManager::getWeatherEnabled()
{
    return data.weatherEnabled;
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

int SettingsManager::getNumPrinters()
{
    return data.numPrinters;
}

OctoPrinterData* SettingsManager::getPrinterData(int printerNum)
{
    return printersData[printerNum];
}

void SettingsManager::setPrinterData(int printerNum, String address, int port, String userName, String password, String apiKey, String displayName)
{
    OctoPrinterData* data = printersData[printerNum];

    data->address = address;
    data->port = port;
    data->username = userName;
    data->password = password;
    data->apiKey = apiKey;
    data->displayName = displayName;
}

void SettingsManager::addNewPrinter(String address, int port, String userName, String password, String apiKey, String displayName, bool enabled)
{
    OctoPrinterData* newPrinter = printersData[data.numPrinters];

    newPrinter->address = address;
    newPrinter->port = port;
    newPrinter->username = userName;
    newPrinter->password = password;
    newPrinter->apiKey = apiKey;
    newPrinter->displayName = displayName;
    newPrinter->enabled = enabled;
    data.numPrinters++;
    updateSettings();
}

void SettingsManager::editPrinter(int printerNum, String address, int port, String userName, String password, String apiKey, String displayName, bool enabled)
{
    OctoPrinterData* printer = printersData[printerNum];

    printer->address = address;
    printer->port = port;
    printer->username = userName;
    printer->password = password;
    printer->apiKey = apiKey;
    printer->displayName = displayName;
    printer->enabled = enabled;

    updateSettings();
}

void SettingsManager::deletePrinter(int printerNum)
{
    for (int i = printerNum; i <= data.numPrinters ; ++i)
    {
        printersData[i] = printersData[i + 1];
    }
    data.numPrinters--;
    updateSettings();
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

