#ifndef _DisplayBase_h
#define _DisplayBase_h

#include "OpenWeatherMapCurrent.h"
#include "OctoPrintMonitor.h"

enum DisplayMode
{
    DisplayMode_Weather,        // detailed current
    DisplayMode_PrintMonitor,   // print monitor
};

class DisplayBase
{
    public:
        DisplayBase() {};

        virtual void drawStartupDisplay() {};
        virtual void startMainDisplay() {};
        virtual void restartMainDisplay() {};

        virtual void drawIPAddress(String ipAddress);
        virtual void drawCurrentTime(unsigned long epochTime) {};        
        virtual void drawCurrentWeather(OpenWeatherMapCurrentData* currentWeather, bool enabled) {};
        virtual void drawWiFiStrength(long dBm) {};
        virtual void drawOctoPrintStatus(OctoPrintMonitorData* printData, String printerName, bool enabled) {};
        
        virtual void serveScreenShot() {};
        virtual void setDisplayBrightness(int percent) {};        
        virtual void setDisplayMode(DisplayMode mode);
        DisplayMode getDisplayMode();

        void setDisplayMetric(bool metric) { displayMetric = metric; }
        bool getDisplayMetric() { return displayMetric; }
    private:
        DisplayMode displayMode = DisplayMode_Weather;
        bool displayMetric = true;
};


#endif // _DisplayBase_h
