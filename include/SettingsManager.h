#ifndef _settingsmanager_h
#define _settingsmanager_h

#include <ArduinoJson.h>
#include "DisplayBase.h"
#include "Settings.h"

const String SETTINGS_FILE_NAME = "/Settings.json";
const int SETTINGS_JSON_SIZE = 1536;

typedef struct SettingsData
{
    String openWeatherMapAPIKey;
    String openWeatherLocationID;

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

    bool octoPrintMonitorEnabled;

    long utcOffsetSeconds;

} SettingsData;

class SettingsManager
{
    public:
        void init();

        void resetSettings();

        String getOpenWeatherApiKey();
        void setOpenWeatherApiKey(String apiKey);

        String getOpenWeatherlocationID();
        void setOpenWeatherlocationID(String locationID);

        DisplayMode getDisplayMode();
        void setDisplayMode(DisplayMode displayMode);

        bool getDisplayMetric();
        void setDisplayMetric(bool metric);

        int getCurrentWeatherInterval();
        void setCurrentWeatherInterval(int interval);

        int getPrintMonitorInterval();
        void setPrintMonitorInterval(int interval);

        String getOctoPrintAddress();
        void setOctoPrintAddress(String address);

        int getOctoPrintPort();
        void setOctoPrintPort(int port);

        String getOctoPrintUsername();
        void setOctoPrintUsername(String userName);

        String getOctoPrintPassword();
        void setOctoPrintPassword(String password);

        String getOctoPrintAPIKey();
        void setOctoPrintAPIKey(String apiKey);

        String getOctoPrintDisplayName();
        void setOctoPrintDisplayName(String displayName);

        bool getOctoPrintEnabled();
        void setOctoPrintEnabled(bool enabled);

        long getUtcOffset();
        void setUtcOffset(long utcOffset);

        int getDisplayBrightness();
        void setDisplayBrightness(int brightnessPercent);

        bool getSettingsChanged();
        void resetSettingsChanged();

    private:
        SettingsData data;
        bool settingsChanged;

        void loadSettings();
        void saveSettings();
        void updateSettings();
};

#endif // _settingsmanager_h