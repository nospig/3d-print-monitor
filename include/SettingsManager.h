#ifndef _settingsmanager_h
#define _settingsmanager_h

#include <ArduinoJson.h>
#include "DisplayBase.h"
#include "Settings.h"

const String SETTINGS_FILE_NAME = "/Settings.json";

#define WEATHER_DISPLAY_SETTING 0
#define CYCLE_DISPLAY_SETTING   -1

typedef struct SettingsData
{
    String openWeatherMapAPIKey;
    String openWeatherLocationID;
    bool weatherEnabled;

    int displayBrightness;
    bool displayMetric;

    int currentWeatherInterval;
    int printMonitorInterval;
    int displayCycleInterval;

    String octoPrintAddress;
    int octoPrintPort;
    String octoPrintUsername;
    String octoPrintPassword;
    String octoPrintAPIKey;
    String octoPrintDisplayName;

    int numPrinters;

    long utcOffsetSeconds;
    ClockFormat clockFormat;
    DateFormat dateFormat;

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

        bool getDisplayMetric();
        void setDisplayMetric(bool metric);

        int getCurrentWeatherInterval();
        void setCurrentWeatherInterval(int interval);
        
        int getPrintMonitorInterval();
        void setPrintMonitorInterval(int interval);

        int getDisplayCycleInterval();
        void setDisplayCycleInterval(int interval);

        int getNumPrinters();
        int getNumEnabledPrinters();
        OctoPrinterData* getPrinterData(int printerNum);
        void addNewPrinter(String address, int port, String userName, String password, String apiKey, String displayName, bool enabled);
        void editPrinter(int printerNum, String address, int port, String userName, String password, String apiKey, String displayName, bool enabled);
        void deletePrinter(int printerNum);

        long getUtcOffset();
        void setUtcOffset(long utcOffset);

        int getDisplayBrightness();
        void setDisplayBrightness(int brightnessPercent);

        int getCurrentDisplay();
        void setCurrentDisplay(int currentDisplay);

        ClockFormat getClockFormat();
        void setClockFormat(ClockFormat clockFormat);

        DateFormat getDateFormat();
        void setDateFormat(DateFormat dateFormat);

        void setSettingsChangedCallback(void(* callback)());
        void setPrinterDeletedCallback(void(* callback)());

    private:
        SettingsData data;
        OctoPrinterData* printersData[MAX_PRINTERS];

        void (* settingsChangedCallback)();
        void (* printerDeletedCallback)();

        void loadSettings();
        void saveSettings();
        void updateSettings();

        void loadPrinters();
        void savePrinters();
};

#endif // _settingsmanager_h