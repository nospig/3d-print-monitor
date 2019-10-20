#ifndef _display_tft_h
#define _display_tft_h

#include "DisplayBase.h"
#include "SPI.h"
#include "TFT_eSPI.h"

// http://rinkydinkelectronics.com/calc_rgb565.php

#define BACKGROUND_COLOUR                   TFT_BLACK
#define SECTION_HEADER_COLOUR               0xFDE0
#define SECTION_HEADER_LINE_COLOUR          0x7518
#define TIME_TEXT_COLOUR                    0x5C55
#define WIFI_STRENGTH_COLOUR                0x7518
#define CURRENT_WEATHER_CONDITIONS_COLOUR   0xFDE0
#define TEMPERATURE_COLOUR                  0xFB20  // blaze orange
#define DETAILED_WEATHER_DESCRIPTION_COLOUR 0xFDE0
#define DETAILED_WEATHER_INFO_COLOUR        0xFB20

#define PRINT_MONITOR_TEXT_COLOUR                       0xFDE0
#define PRINT_MONITOR_ARC_COLOUR                        TFT_RED
#define PRINT_MONITOR_ARC_BACKGROUND_COLOUR             TFT_DARKGREY
#define PRINT_MONITOR_ACTUAL_TEMP_COLOUR                TEMPERATURE_COLOUR
#define PRINT_MONITOR_TARGET_TEMP_COLOUR                TEMPERATURE_COLOUR
#define PRINT_MONITOR_TEMP_HEADING_COLOUR               0x75DF
#define PRINT_MONITOR_ARC_TARGET_COLOUR                 TFT_DARKGREY
#define PRINT_MONITOR_PRINTER_NAME_COLOUR               0xFDE0
#define PRINT_MONITOR_PROGRESS_BAR_COLOUR               TFT_GREEN
#define PRINT_MONITOR_PROGRESS_BAR_BACKGROUND_COLOUR    TFT_DARKGREY
#define PRINT_MONITOR_PROGRESS_BAR_OUTLINE_COLOUR       TFT_WHITE
#define PRINT_MONITOR_PROGRESS_COLOUR                   0xFDE0
#define PRINT_MONITOR_JOB_INFO_HEADING_COLOUR           0xFDE0
#define PRINT_MONITOR_JOB_INFO_COLOUR                   TFT_WHITE

#define TIME_HEIGHT  20
#define TIME_Y       300


#define TOOL_TEMP_DISPLAY_X             60
#define TOOL_TEMP_DISPLAY_Y             90
#define BED_TEMP_DISPLAY_X              240 - TOOL_TEMP_DISPLAY_X
#define BED_TEMP_DISPLAY_Y              TOOL_TEMP_DISPLAY_Y
#define PRINT_INFO_SECTION_DIVIDER_Y    140
#define PRINT_PROGRESS_BAR_WIDTH        160
#define PRINT_PROGRESS_BAR_HEIGHT       10

#define WEATHER_ICON_WIDTH  48
#define WEATHER_ICON_HEIGHT 48

#define PIXEL_TIMEOUT 100     // 100ms Time-out between pixel requests
#define START_TIMEOUT 10000   // 10s Maximum time to wait at start transfer

#define BITS_PER_PIXEL 16     // 24 for RGB colour format, 16 for 565 colour format

// File names must be alpha-numeric characters (0-9, a-z, A-Z) or "/" underscore "_"
// other ascii characters are stripped out by client, including / generates
// sub-directories
#define DEFAULT_FILENAME "tft_screenshots/screenshot" // In case none is specified
#define FILE_TYPE "png"       // jpg, bmp, png, tif are valid


// Filename extension
// '#' = add 0-9, '@' = add timestamp, '%' add millis() timestamp, '*' = add nothing
// '@' and '%' will generate new unique filenames, so beware of cluttering up your
// hard drive with lots of images! The PC client sketch is set to limit the number of
// saved images to 1000 and will then prompt for a restart.
#define FILE_EXT  '%'         

// Number of pixels to send in a burst (minimum of 1), no benefit above 8
// NPIXELS values and render times: 1 = 5.0s, 2 = 1.75s, 4 = 1.68s, 8 = 1.67s
#define NPIXELS 8  // Must be integer division of both TFT width and TFT height

#define BRIGHTNESS_PIN          16   // D0
#define MAX_BRIGHTNESS_VALUE    1023 // max value of analog out on ESP8266

#define DEG2RAD 0.0174532925
#define TEMP_ARC_START              240
#define TEMP_ARC_SPAN               240
#define TEMP_ARC_DEGREE_PER_SEG     3
#define TOOL_TEMP_MAX               250
#define BED_TEMP_MAX                100

class DisplayTFT : public DisplayBase
{
    public:
        DisplayTFT();

        void drawStartupDisplay();
        void clearDisplay();
        void drawIPAddress(String ipAddress);

        void drawCurrentTime(unsigned long epochTime, ClockFormat clockFormat, DateFormat dateFormat);
        void drawCurrentWeather(OpenWeatherMapCurrentData* currentWeather, bool enabled);
        void drawWiFiStrength(long dBm);
        void drawOctoPrintStatus(OctoPrintMonitorData* printData, String printerName, bool enabled);    

        void setDisplayMode(DisplayMode mode);
        void serveScreenShot();
        void setDisplayBrightness(int percent);
        
    private:
        int drawCurrentWeather(OpenWeatherMapCurrentData* currentWeather, int y);
        void drawDetailedCurrentWeather(OpenWeatherMapCurrentData* currentWeather, int y);
        void drawWeatherNotEnabled();
        const unsigned short* getIconData(String iconId);
        void drawTimeDisplay(unsigned long epochTime, ClockFormat clockFormat, DateFormat dateFormat, int y);

        void drawInvalidPrintData(String printerName);
        void drawPrinterNotEnabled(String printerName);
        void drawPrintInfo(OctoPrintMonitorData* printData, String printerName);
        void drawTempArc(String title, float value, float target, float max, int x, int y);
        String getPrintInfoTitle(String printerName, uint16_t flags);
        void drawJobInfo(OctoPrintMonitorData* printData, int y);
        void drawProgressBar(float percent, int x, int y, int width, int height, uint32_t barColour, uint32_t backgroundColour);
        void formatSeconds(char* buffer, int seconds);

        int fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
        char* getTempPostfix();

        boolean screenServer(void);
        boolean screenServer(String filename);
        boolean serialScreenServer(String filename);
        void sendParameters(String filename);

        bool showingPrintInfo;
        bool showingNoPrintInfo;
        bool showingNotEnabled;

        TFT_eSPI *tft;
        int brightness;
};

#endif // _display_tft_h