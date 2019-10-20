#ifndef _DisplayBase_h
#define _DisplayBase_h

#include "OpenWeatherMapCurrent.h"
#include "OctoPrintMonitor.h"

enum DisplayMode
{
    DisplayMode_Weather,        // detailed current
    DisplayMode_PrintMonitor,   // print monitor
    DisplayMode_NotSetup,
};

enum ClockFormat
{
    ClockFormat_24h,       
    ClockFormat_AmPm,  
};

enum DateFormat
{
    DateFormat_DDMMYY,       
    DateFormat_MMDDYY,  
};

class DisplayBase
{
    public:
        DisplayBase() {};

        virtual void drawStartupDisplay() {};
        virtual void clearDisplay() {};

        virtual void drawCurrentTime(unsigned long epochTime, ClockFormat clockFormat, DateFormat dateFormat) {};        
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
