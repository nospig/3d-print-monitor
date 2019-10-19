#ifndef _settingsmanager_h
#define _settingsmanager_h

#include <ArduinoJson.h>
#include "DisplayBase.h"
#include "Settings.h"

const String SETTINGS_FILE_NAME = "/Settings.json";

typedef struct SettingsData
{
    String openWeatherMapAPIKey;
    String openWeatherLocationID;
    bool weatherEnabled;

    DisplayMode dispayMode;
    int displayBrightness;
    bool displayMetric;

    int currentWeatherInterval;
    int printMonitorInterval;

    String octoPrintAddress;
    int octoPrintPort;
    String octoPrintUsername;
    String octoPrintPassword;
    String octoPrintAPIKey;
    String octoPrintDisplayName;

    int numPrinters;
    bool octoPrintMonitorEnabled;

    long utcOffsetSeconds;

    int currentDisplay;

} SettingsData;

typedef struct OctoPrinterData
{
    String address;
    int port;
    String username;
    String password;
    String apiKey;
    String displayName;
    bool enabled;
} OctoPrinterData;

class SettingsManager
{
    public:
        void init();

        void resetSettings();

        String getOpenWeatherApiKey();
        void setOpenWeatherApiKey(String apiKey);

        String getOpenWeatherlocationID();
        void setOpenWeatherlocationID(String locationID);

        void setWeatherEnabled(bool enabled);
        bool getWeatherEnabled();

        DisplayMode getDisplayMode();
        void setDisplayMode(DisplayMode displayMode);

        bool getDisplayMetric();
        void setDisplayMetric(bool metric);

        int getCurrentWeatherInterval();
        void setCurrentWeatherInterval(int interval);
        
        int getPrintMonitorInterval();
        void setPrintMonitorInterval(int interval);

        int getNumPrinters();
        OctoPrinterData* getPrinterData(int printerNum);
        void setPrinterData(int printerNum, String address, int port, String userName, String password, String apiKey, String displayName);
        void addNewPrinter(String address, int port, String userName, String password, String apiKey, String displayName, bool enabled);
        void editPrinter(int printerNum, String address, int port, String userName, String password, String apiKey, String displayName, bool enabled);
        void deletePrinter(int printerNum);
        bool getOctoPrintEnabled();
        void setOctoPrintEnabled(bool enabled);

        long getUtcOffset();
        void setUtcOffset(long utcOffset);

        int getDisplayBrightness();
        void setDisplayBrightness(int brightnessPercent);

        int getCurrentDisplay();
        void setCurrentDisplay(int currentDisplay);

        bool getSettingsChanged();
        void resetSettingsChanged();

    private:
        SettingsData data;
        bool settingsChanged;

        OctoPrinterData* printersData[MAX_PRINTERS];

        void loadSettings();
        void saveSettings();
        void updateSettings();

        void loadPrinters();
        void savePrinters();
};

#endif // _settingsmanager_h