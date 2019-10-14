#ifndef _WebServer_H_
#define _WebServer_H_

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "OpenWeatherMapCurrent.h"
#include "OctoPrintMonitor.h"
#include "SettingsManager.h"

class WebServer
{
    public:
        void init(SettingsManager* settingsManager);
        static AsyncWebServer* getServer();

        void updateCurrentWeather(OpenWeatherMapCurrentData* currentWeather);
        void updatePrintMonitorInfo(OctoPrintMonitorData* printerInfo, bool enabled);

        bool screenGrabRequested();
        void clearScreenGrabRequest();

    private:
        static void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
        static void updateClientOnConnect();
        static String tokenProcessor(const String& token);
        static void handleUpdateWeatherSettings(AsyncWebServerRequest* request);
        static void handleUpdateDisplaySettings(AsyncWebServerRequest* request);       
        static void handleUpdateTimings(AsyncWebServerRequest* request);
        static void handleUpdateClockSettings(AsyncWebServerRequest* request);        
        
        static void handleUpdatePrintMonitorSettings(AsyncWebServerRequest* request);     
        static void handleAddNewPrinter(AsyncWebServerRequest* request);
        static void handleDeletePrinter(AsyncWebServerRequest* request);
        static void handleEditPrinter(AsyncWebServerRequest* request);
        static void handleGetPrinter(AsyncWebServerRequest* request);    

        static void handleForgetWiFi(AsyncWebServerRequest* request);
        static void handleResetSettings(AsyncWebServerRequest* request);
        static void handleScreenGrab(AsyncWebServerRequest* request);
        static String createPrinterList();

        static String currentWeatherJson;
        static String currentPrinterJson;
        static bool screenGrabRequest;
        
        static SettingsManager* settingsManager;    
};

#endif