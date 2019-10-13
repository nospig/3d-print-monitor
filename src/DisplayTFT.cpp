#include <Arduino.h>
#include "DisplayTFT.h"
#include "SPI.h"
#include "TFT_eSPI.h"
#include <time.h>
#include "icons/weatherIcons.h"
#include "Settings.h"

const char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

DisplayTFT::DisplayTFT()
{
    tft = new TFT_eSPI();

    tft->begin();
    tft->setRotation(SCREEN_ROTATION);

    // Swap the colour byte order when rendering for images
    tft->setSwapBytes(true);

    // if dimming is setup, TODO
    analogWrite(BRIGHTNESS_PIN, brightness);
    tft->writecommand(0x11);

    showingPrintInfo = false;
    showingNoPrintInfo = false;
    showingNotEnabled = false;
}
 
void DisplayTFT::setDisplayBrightness(int percent)
{
    brightness = (MAX_BRIGHTNESS_VALUE * percent) / 100;
    analogWrite(BRIGHTNESS_PIN, brightness);
}

void DisplayTFT::drawStartupDisplay()
{
    tft->fillScreen(BACKGROUND_COLOUR);

    tft->setTextFont(4);
    tft->setTextDatum(BC_DATUM);
    tft->setTextColor(TFT_WHITE, BACKGROUND_COLOUR); 
    tft->drawString("Connecting.", tft->width()/2, tft->height()/2);
}

void DisplayTFT::startMainDisplay()
{
    tft->fillScreen(BACKGROUND_COLOUR);
    drawStaticElements();
}

void DisplayTFT::drawIPAddress(String ipAddress)
{
    char buffer[64];

    tft->setTextFont(2);
    tft->setTextColor(TFT_WHITE, BACKGROUND_COLOUR); 
   
    tft->setTextDatum(BC_DATUM);
    sprintf(buffer, "IP Adresss: %s", ipAddress.c_str());
    tft->drawString(buffer, tft->width()/2, 40); 
}

void DisplayTFT::restartMainDisplay()
{
    startMainDisplay();
}

void DisplayTFT::setDisplayMode(DisplayMode mode)
{
    DisplayBase::setDisplayMode(mode);

    showingPrintInfo = false;
    showingNoPrintInfo = false;

    // do display changing logic, going to assume for now in main display mode
    // caller responsible for updating all elements after making this call
    tft->fillScreen(BACKGROUND_COLOUR);
    drawStaticElements();
}

void DisplayTFT::drawCurrentTime(unsigned long epochTime)
{
    drawTimeDisplay(epochTime, TIME_Y);
}

void DisplayTFT::drawCurrentWeather(OpenWeatherMapCurrentData* currentWeather)
{
    switch(getDisplayMode())
    {
        case DisplayMode_Weather:
            drawDetailedCurrentWeather(currentWeather, 0);
        default:
            break;
    }    
}

void DisplayTFT::drawWiFiStrength(long dBm)
{
    int percentage = min(max(2 * ((int)dBm + 100), 0), 100); // how Microsoft convert, linear in the range -100 to -50

    int x = 4;
    int y = tft->height()-2;
    uint32_t barColour;
    int barValue = 0;
    int barHeight = 2;

    for(int i=0; i<5; i++)
    {
        if(percentage >= barValue)
        {
            barColour = WIFI_STRENGTH_COLOUR;
        }
        else
        {
            barColour = BACKGROUND_COLOUR;
        }
        tft->drawLine(x, y, x, y-barHeight, barColour);
        
        barHeight += 2;
        barValue += 20;
        x += 2;
    }
}

/****************************************************************************************
 * 
 *  Drawing routines
 * 
****************************************************************************************/

int DisplayTFT::drawCurrentWeather(OpenWeatherMapCurrentData* currentWeather, int y)
{
    // maybe best just to wipe as not updated often
    tft->fillRect(0, y+20, tft->width(), 80, BACKGROUND_COLOUR);

    if(currentWeather->validData)
    {       
        tft->setTextFont(2);
        tft->setTextDatum(TC_DATUM);
        tft->setTextColor(SECTION_HEADER_COLOUR); 
        tft->drawString(currentWeather->location, tft->width()/2, y+2); 

        tft->setTextFont(4);
        tft->setTextColor(TEMPERATURE_COLOUR); 

        String tempString = String(currentWeather->temp, 1);
        int x = tft->width()/2 - 40;
        int widthTemp;

        tft->setTextDatum(TR_DATUM);
        tft->drawString(tempString + getTempPostfix(), x , y + 40);    
        widthTemp = tft->textWidth(tempString + getTempPostfix());

        String description = currentWeather->description;

        bool truncatedDescription = false;

        tft->setTextFont(2);
        tft->setTextColor(CURRENT_WEATHER_CONDITIONS_COLOUR); 
        tft->setTextDatum(TL_DATUM);

        while(tft->textWidth(description) > (tft->width() - (x - widthTemp)))
        {
            truncatedDescription = true;
            description = description.substring(0, description.length()-1);
        }

        if(truncatedDescription)
        {
            description = description + "...";
        }
        tft->drawString(description, x - widthTemp, y + 82);    

        tft->pushImage(160, y+30, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, getIconData(currentWeather->icon));

        return x - widthTemp;
    }

    return 0;
}

void DisplayTFT::drawStaticElements()
{
    switch(getDisplayMode())
    {
        case DisplayMode_Weather:
            //tft->drawLine(0, MODE_1_CURRENT_Y, tft->width(), MODE_1_CURRENT_Y, SECTION_HEADER_LINE_COLOUR);
            tft->drawLine(0, TIME_Y, tft->width(), TIME_Y, SECTION_HEADER_LINE_COLOUR); 
            break;
        case DisplayMode_PrintMonitor:
            tft->drawLine(0, TIME_Y, tft->width(), TIME_Y, SECTION_HEADER_LINE_COLOUR); 
            break;
        default:
            break;
    }
}

void DisplayTFT::drawTimeDisplay(unsigned long epochTime, int y)
{
    tft->setTextFont(2);
    tft->setTextColor(TIME_TEXT_COLOUR, BACKGROUND_COLOUR); 
   
    time_t time = epochTime;
    struct tm* timeInfo;
    timeInfo = gmtime(&time);
    char buffer[16];

    y += TIME_HEIGHT;

    tft->setTextDatum(BR_DATUM);
    tft->setTextPadding(tft->textWidth("23:59"));
    sprintf(buffer, "%02d:%02d\n", timeInfo->tm_hour, timeInfo->tm_min);
    tft->drawString(buffer, tft->width()/2-30, y); 

    tft->setTextDatum(BC_DATUM);
    tft->setTextPadding(tft->textWidth(daysOfTheWeek[3]));  // Wed longest?
    tft->drawString(daysOfTheWeek[timeInfo->tm_wday], tft->width()/2, y); 

    tft->setTextDatum(BL_DATUM);
    tft->setTextPadding(tft->textWidth("31/12/99"));
    sprintf(buffer, "%d/%d/%02d\n", timeInfo->tm_mday, timeInfo->tm_mon+1, (timeInfo->tm_year+1900) % 100);
    tft->drawString(buffer, tft->width()/2+30, y); 
}

void DisplayTFT::drawDetailedCurrentWeather(OpenWeatherMapCurrentData* currentWeather, int y)
{
    int x, width;
    char buffer[64];
    time_t time;
    struct tm* timeInfo;

    x = drawCurrentWeather(currentWeather, y);

    y += 130;

    tft->drawLine(0, y, tft->width(), y, SECTION_HEADER_LINE_COLOUR); 

    y += 15;

    tft->fillRect(0, y, tft->width(), 150, BACKGROUND_COLOUR);

    if(currentWeather->validData)
    {
        tft->setTextFont(2);
        tft->setTextDatum(TL_DATUM);

        // min and max temps
        sprintf(buffer, "Min | Max: ");
        width = tft->textWidth(buffer);
        tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
        tft->drawString(buffer, x, y);    

        sprintf(buffer, "%.1f%s | Max: %.1f%s", currentWeather->tempMin, getTempPostfix(), currentWeather->tempMax, getTempPostfix());
        tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
        tft->drawString(buffer, x + width, y);    

        y += tft->fontHeight();

        // humidity
        sprintf(buffer, "Humidity: ");
        width = tft->textWidth(buffer);
        tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
        tft->drawString(buffer, x, y);    
        
        sprintf(buffer, "%d%%", currentWeather->humidity);
        tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
        tft->drawString(buffer, x + width, y);    
        
        y += tft->fontHeight();

        // pressure
        tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
        sprintf(buffer, "Pressure: ");
        width = tft->textWidth(buffer);
        tft->drawString(buffer, x, y);    

        if(getDisplayMetric())
        {
            sprintf(buffer, "%d hpa", currentWeather->pressure);
        }
        else
        {     
            sprintf(buffer, "%d mb", currentWeather->pressure);
        }
        tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
        tft->drawString(buffer, x + width, y);    

        y += tft->fontHeight();

        // wind
        tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
        sprintf(buffer, "Wind: ");
        width = tft->textWidth(buffer);
        tft->drawString(buffer, x, y);    

        if(getDisplayMetric())
        {
            sprintf(buffer, "%.1fm/s from %.0f degress", currentWeather->windSpeed, currentWeather->windDeg);
        }
        else
        {
            sprintf(buffer, "%.1fmph from %.0f degress", currentWeather->windSpeed, currentWeather->windDeg);
        }
        
        tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
        tft->drawString(buffer, x + width, y);    

        y += tft->fontHeight();

        // clouds
        if(currentWeather->cloudPercentage > -1)
        {
            tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
            sprintf(buffer, "Clouds: ");
            width = tft->textWidth(buffer);
            tft->drawString(buffer, x, y);    

            sprintf(buffer, "%d%%", currentWeather->cloudPercentage);
            tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
            tft->drawString(buffer, x + width, y);    
            y += tft->fontHeight();
        }

        // rain
        if(currentWeather->rainOneHour > -1)
        {
            tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
            sprintf(buffer, "Rain: ");
            width = tft->textWidth(buffer);
            tft->drawString(buffer, x, y);    

            sprintf(buffer, "1h %dmm, 3h %dmm", currentWeather->rainOneHour, currentWeather->rainThreeHour);
            tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
            tft->drawString(buffer, x + width, y);    
  
            y += tft->fontHeight();
        }
        
        // sunrise
        time = currentWeather->sunRise + currentWeather->timeZone;
        timeInfo = gmtime(&time);

        tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
        sprintf(buffer, "Sunrise: ");
        width = tft->textWidth(buffer);
        tft->drawString(buffer, x, y);    

        sprintf(buffer, "%02d:%02d\n", timeInfo->tm_hour, timeInfo->tm_min);
        tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
        tft->drawString(buffer, x + width, y);   
        y += tft->fontHeight();

        // setset
        time = currentWeather->sunSet + currentWeather->timeZone;
        timeInfo = gmtime(&time);

        tft->setTextColor(DETAILED_WEATHER_DESCRIPTION_COLOUR); 
        sprintf(buffer, "Sunset: ");
        width = tft->textWidth(buffer);
        tft->drawString(buffer, x, y);    

        sprintf(buffer, "%02d:%02d\n", timeInfo->tm_hour, timeInfo->tm_min);
        tft->setTextColor(DETAILED_WEATHER_INFO_COLOUR); 
        tft->drawString(buffer, x + width, y);   
        y += tft->fontHeight();
    }
}

/****************************************************************************************
 * 
 *  Print monitor display
 * 
 * 
****************************************************************************************/

void DisplayTFT::drawOctoPrintStatus(OctoPrintMonitorData* printData, String printerName, bool enabled)
{
    if(getDisplayMode() != DisplayMode_PrintMonitor)
    {
        return;
    }

    if(!enabled)
    {
        showingPrintInfo = false;
        showingNoPrintInfo = false;

        if(!showingNotEnabled)
        {
            tft->fillRect(0, 0, tft->width(), TIME_Y-1, BACKGROUND_COLOUR);
        }
        drawOctoPrintNotEnabled();
        showingNotEnabled = true;
    }
    else
    {
        if(!printData->validPrintData)
        {
            showingPrintInfo = false;
            if(!showingNoPrintInfo)
            {
                tft->fillRect(0, 0, tft->width(), TIME_Y-1, BACKGROUND_COLOUR);
                showingNoPrintInfo = true;
            }
            drawInvalidPrintData(printerName);
        }
        else
        {
            showingNoPrintInfo = false;
            if(!showingPrintInfo)
            {
                tft->fillRect(0, 0, tft->width(), TIME_Y-1, BACKGROUND_COLOUR);
                showingPrintInfo = true;
            }
            drawPrintInfo(printData, printerName);
        }
    }
}

void DisplayTFT::drawInvalidPrintData(String printerName)
{
    tft->setTextDatum(MC_DATUM);
    tft->setTextFont(2);
    tft->setTextColor(PRINT_MONITOR_TEXT_COLOUR, BACKGROUND_COLOUR); 
    tft->drawString("No printer data available", tft->width()/2, tft->height()/4);   

    tft->setTextDatum(TC_DATUM);
    tft->setTextColor(PRINT_MONITOR_PRINTER_NAME_COLOUR, BACKGROUND_COLOUR); 

    tft->drawString(printerName, tft->width()/2, TOOL_TEMP_DISPLAY_Y - 88);  
}

void DisplayTFT::drawOctoPrintNotEnabled()
{
    if(getDisplayMode() == DisplayMode_PrintMonitor)
    {
        tft->setTextDatum(MC_DATUM);
        tft->setTextFont(2);
        tft->setTextColor(PRINT_MONITOR_TEXT_COLOUR, BACKGROUND_COLOUR); 
        tft->drawString("Enable OctoPrint monitor", tft->width()/2, tft->height()/4);   
        tft->drawString("in settings", tft->width()/2, (tft->height()/4) + tft->fontHeight());  
    }
}

void DisplayTFT::drawPrintInfo(OctoPrintMonitorData* printData, String printerName)
{
    tft->setTextFont(2);
    tft->setTextDatum(TC_DATUM);
    tft->setTextColor(PRINT_MONITOR_PRINTER_NAME_COLOUR, BACKGROUND_COLOUR); 
    tft->setTextPadding(tft->width());  

    String printer = printerName;
    if(printer == "")
    {
        printer = "Printer";
    }
     
    String title = getPrintInfoTitle(printer, printData->printerFlags);
    tft->drawString(title, tft->width()/2, TOOL_TEMP_DISPLAY_Y - 88); 

    drawTempArc("Tool", printData->tool0Temp, printData->tool0Target, TOOL_TEMP_MAX, TOOL_TEMP_DISPLAY_X, TOOL_TEMP_DISPLAY_Y);
    drawTempArc("Bed", printData->bedTemp, printData->bedTarget, BED_TEMP_MAX, BED_TEMP_DISPLAY_X, BED_TEMP_DISPLAY_Y);

    tft->drawLine(0, PRINT_INFO_SECTION_DIVIDER_Y, tft->width(), PRINT_INFO_SECTION_DIVIDER_Y, SECTION_HEADER_LINE_COLOUR);

    if(printData->jobLoaded)
    {
        drawJobInfo(printData, PRINT_INFO_SECTION_DIVIDER_Y);
    }
    else
    {
        tft->fillRect(0, PRINT_INFO_SECTION_DIVIDER_Y + 1, tft->width(), TIME_Y - PRINT_INFO_SECTION_DIVIDER_Y - 1, BACKGROUND_COLOUR);
    }   
}

void DisplayTFT::drawJobInfo(OctoPrintMonitorData* printData, int y)
{
    int x;
    char estimatedTimeBuffer[32];
    char buffer[128];
    char timeBuffer[32];
    int infoX, elapsedPadding;
    
    x = (tft->width() / 2) - (PRINT_PROGRESS_BAR_WIDTH / 2);
    y += 15;
    
    drawProgressBar(printData->percentComplete, x, y, PRINT_PROGRESS_BAR_WIDTH, PRINT_PROGRESS_BAR_HEIGHT, 
        PRINT_MONITOR_PROGRESS_BAR_COLOUR, PRINT_MONITOR_PROGRESS_BAR_BACKGROUND_COLOUR);

    y += PRINT_PROGRESS_BAR_HEIGHT;
    y += 15;
    x = 20;

    tft->setTextFont(2);
    tft->setTextDatum(TL_DATUM);

    // estimated time
    tft->setTextColor(PRINT_MONITOR_JOB_INFO_HEADING_COLOUR, BACKGROUND_COLOUR); 
    infoX = x + tft->drawString("Estimated time: ", x, y);        
    
    sprintf(buffer, "999:59:59");
    tft->setTextPadding(tft->textWidth(buffer));
    tft->setTextColor(PRINT_MONITOR_JOB_INFO_COLOUR, BACKGROUND_COLOUR); 
    formatSeconds(estimatedTimeBuffer, (int)printData->estimatedPrintTime);
    sprintf(buffer, "%s", estimatedTimeBuffer);
    elapsedPadding = tft->textWidth(estimatedTimeBuffer);
    tft->drawString(buffer, infoX, y);    
    y += tft->fontHeight();

    // elapsed print time
    tft->setTextColor(PRINT_MONITOR_JOB_INFO_HEADING_COLOUR, BACKGROUND_COLOUR); 
    infoX = x + tft->drawString("Print time: ", x, y);    

    tft->setTextColor(PRINT_MONITOR_JOB_INFO_COLOUR, BACKGROUND_COLOUR); 
    tft->setTextPadding(elapsedPadding);

    if(printData->printTimeElapsed > 0.0f)
    {
        formatSeconds(timeBuffer, (int)printData->printTimeElapsed);
        tft->drawString(timeBuffer, infoX, y);
    }
    else
    {
        tft->drawString("-", infoX, y);    
    }    
    y += tft->fontHeight();

    // remaining
    tft->setTextColor(PRINT_MONITOR_JOB_INFO_HEADING_COLOUR, BACKGROUND_COLOUR); 
    infoX = x + tft->drawString("Remaining time: ", x, y);    

    tft->setTextColor(PRINT_MONITOR_JOB_INFO_COLOUR, BACKGROUND_COLOUR); 

    if(printData->printTimeElapsed > 0.0f)
    {
        float remaining = printData->printTimeRemaining;
        remaining = max(0.0f, remaining);
        formatSeconds(timeBuffer, (int)remaining);
        tft->drawString(timeBuffer, infoX, y);
    }
    else
    {
        tft->drawString("-", infoX, y);    
    }
    y += tft->fontHeight();

    // filament length
    int padding;
    
    tft->setTextColor(PRINT_MONITOR_JOB_INFO_HEADING_COLOUR, BACKGROUND_COLOUR); 
    infoX = x + tft->drawString("Filament: ", x, y);    

    tft->setTextColor(PRINT_MONITOR_JOB_INFO_COLOUR, BACKGROUND_COLOUR); 

    sprintf(buffer, "9999.9m");
    padding = tft->textWidth(buffer);
    tft->setTextPadding(padding);

    sprintf(buffer, "%.02fm", printData->filamentLength / 1000.0f);
    tft->drawString(buffer, infoX, y);

    // file name
    y += tft->fontHeight();

    tft->setTextColor(PRINT_MONITOR_JOB_INFO_HEADING_COLOUR, BACKGROUND_COLOUR); 
    tft->drawString("File:", x, y);
    y += tft->fontHeight();

    tft->setTextColor(PRINT_MONITOR_JOB_INFO_COLOUR, BACKGROUND_COLOUR); 

    String file = printData->fileName;
    bool truncatedDescription = false;

    while(tft->textWidth(file) > (tft->width() - x - 20))
    {
        truncatedDescription = true;
        file = file.substring(0, file.length()-1);
    }

    if(truncatedDescription)
    {
        file = file + "...";
    }

    tft->setTextPadding(tft->width() - x);
    tft->drawString(file, x, y); 
}

void DisplayTFT::formatSeconds(char* buffer, int seconds)
{
    int hours, minutes;

    hours = seconds / 3600;
    seconds = seconds % 3600;
    minutes = seconds / 60;
    seconds = seconds % 60;
    seconds = seconds;

    sprintf(buffer, "%02d:%02d:%02d", hours, minutes, seconds);    
}

void DisplayTFT::drawProgressBar(float percent, int x, int y, int width, int height, uint32_t barColour, uint32_t backgroundColour)
{
    int completedWidth;
    char buffer[16];
    int barX;

    percent = min(percent, 100.0f);
    percent = max(0.0f, percent);
    barX = x + 10;  // space for text

    completedWidth = (PRINT_PROGRESS_BAR_WIDTH * percent) / 100.0f;
    tft->fillRect(barX, y, completedWidth, 10, barColour);
    tft->fillRect(barX + completedWidth, y, PRINT_PROGRESS_BAR_WIDTH - completedWidth, height, backgroundColour);
    tft->drawRect(barX -1, y -1, PRINT_PROGRESS_BAR_WIDTH + 2, height + 2, PRINT_MONITOR_PROGRESS_BAR_OUTLINE_COLOUR);

    tft->setTextFont(2);
    tft->setTextColor(PRINT_MONITOR_PROGRESS_COLOUR, BACKGROUND_COLOUR); 
    tft->setTextDatum(CR_DATUM);
    tft->setTextPadding(tft->textWidth("100%"));
    sprintf(buffer, "%0.f%%", percent);
    tft->drawString(buffer, x, y + (height / 2));
}

String DisplayTFT::getPrintInfoTitle(String printerName, uint16_t flags)
{
    String title;

    if((flags & PRINT_STATE_CLOSED_OR_ERROR) || (flags & PRINT_STATE_ERROR))
    {
        title = printerName + " - Error";
    }
    else if(flags & PRINT_STATE_CANCELLING)
    {
        title = printerName + " - Cancelling";
    }
    else if(flags & PRINT_STATE_FINISHING)
    {
        title = printerName + " - Finishing";
    }
    else if(flags & PRINT_STATE_PAUSING)
    {
        title = printerName + " - Pausing";
    }
    else if(flags & PRINT_STATE_PAUSED)
    {
        title = printerName + " - Paused";
    }    
    else if(flags & PRINT_STATE_RESUMING)
    {
        title = printerName + " - Resuming";
    }
    else if(flags & PRINT_STATE_PRINTING)
    {
        title = printerName + " - Printing";
    }
    else if(flags & PRINT_STATE_READY)
    {
        title = printerName + " - Ready";
    }
    else
    {
        title = printerName;
    }

    return title;
}

void DisplayTFT::drawTempArc(String title, float value, float target, float max, int x, int y)
{
    char buffer[64];
    int endAngle;
    int padding;

    tft->setTextFont(2);
    tft->setTextColor(PRINT_MONITOR_TEMP_HEADING_COLOUR, BACKGROUND_COLOUR); 
    tft->setTextDatum(BC_DATUM);
    padding = tft->textWidth(title);
    tft->setTextPadding(padding);
    tft->drawString(title, x, y - 50);

    tft->setTextFont(4);
    tft->setTextColor(PRINT_MONITOR_ACTUAL_TEMP_COLOUR, BACKGROUND_COLOUR); 
    sprintf(buffer, "%.0fC", max);
    padding = tft->textWidth(buffer);
    tft->setTextPadding(padding);
    tft->setTextDatum(TC_DATUM);

    sprintf(buffer, "%.0fC", value);
    tft->drawString(buffer, x, y + 20);

    tft->setTextFont(2);
    tft->setTextColor(PRINT_MONITOR_TARGET_TEMP_COLOUR, BACKGROUND_COLOUR); 
    sprintf(buffer, "%.0fC", max);
    padding = tft->textWidth(buffer);
    tft->setTextDatum(BC_DATUM);
    sprintf(buffer, "%.0fC", target);
    tft->drawString(buffer, x, y);

    float temp = min(value, max);
    float segments = ((temp / max) * TEMP_ARC_SPAN) / TEMP_ARC_DEGREE_PER_SEG;
    endAngle = fillArc(x, y, TEMP_ARC_START, (int)segments, 40, 40, 8, PRINT_MONITOR_ARC_COLOUR);

    segments = (max - temp) / max;
    segments = (segments * TEMP_ARC_SPAN) / TEMP_ARC_DEGREE_PER_SEG;
    fillArc(x, y, endAngle, (int)segments, 40, 40, 8, PRINT_MONITOR_ARC_BACKGROUND_COLOUR);
}

/****************************************************************************************
 * 
 *  Misc
 * 
 * 
****************************************************************************************/

char* DisplayTFT::getTempPostfix()
{
    static char postFix[2];

    if(getDisplayMetric())
    {
        sprintf(postFix, "C");
    }
    else
    {
        sprintf(postFix, "F");
    }
    return postFix;
}

const unsigned short* DisplayTFT::getIconData(String iconId)
{    
    // convert icon code from weather to our image, not all handled
    if(iconId == "01d")
    {
        return icon_01d;
    }
    if(iconId == "02d")
    {
        return icon_02d;
    }
    if(iconId == "03d")
    {
        return icon_03d;
    }
    if(iconId == "04d")
    {
        return icon_03d;
    }
    if(iconId == "09d")
    {
        return icon_09d;
    }
    if(iconId == "10d")
    {
        return icon_10d;
    }
    if(iconId == "11d")
    {
        return icon_11d;
    }
    if(iconId == "13d")
    {
        return icon_13d;
    }
    if(iconId == "50d")
    {
        return icon_50d;
    }
    if(iconId == "01n")
    {
        return icon_01n;
    }
    if(iconId == "02n")
    {
        return icon_02n;
    }
    if(iconId == "03n")
    {
        return icon_03d;
    }
    if(iconId == "04n")
    {
        return icon_03d;
    }
    if(iconId == "09n")
    {
        return icon_09n;
    }
    if(iconId == "10n")
    {
        return icon_09n;
    }
    if(iconId == "11n")
    {
        return icon_11n;
    }
    if(iconId == "13n")
    {
        return icon_13n;
    }
    if(iconId == "50n")
    {
        return icon_50d;
    }

    Serial.println("Icon: " + iconId + " not handled.");

    return icon_01d;
}

/****************************************************************************************
 * 
 *  Screen server routines
 *  From TFT-eSPI library
 * 
****************************************************************************************/

void DisplayTFT::serveScreenShot()
{
    screenServer();
}

boolean DisplayTFT::screenServer(void)
{
    // With no filename the screenshot will be saved with a default name e.g. tft_screen_#.xxx
    // where # is a number 0-9 and xxx is a file type specified below
    return screenServer(DEFAULT_FILENAME);
}

//====================================================================================
//                           Screen server call with filename
//====================================================================================
// Start a screen dump server (serial or network) - filename specified
boolean DisplayTFT::screenServer(String filename)
{
    boolean result = serialScreenServer(filename); // Screenshot serial port server
    delay(0); // Equivalent to yield() for ESP8266;

    return result;
}

//====================================================================================
//                Serial server function that sends the data to the client
//====================================================================================
boolean DisplayTFT::serialScreenServer(String filename)
{
    // Precautionary receive buffer garbage flush for 50ms
    uint32_t clearTime = millis() + 50;
    while (millis() < clearTime && Serial.read() >= 0)
        delay(0); // Equivalent to yield() for ESP8266;

    boolean wait = true;
    uint32_t lastCmdTime = millis(); // Initialise start of command time-out

    // Wait for the starting flag with a start time-out
    while (wait)
    {
        delay(0); // Equivalent to yield() for ESP8266;
        // Check serial buffer
        if (Serial.available() > 0)
        {
            // Read the command byte
            uint8_t cmd = Serial.read();
            // If it is 'S' (start command) then clear the serial buffer for 100ms and stop waiting
            if (cmd == 'S')
            {
                // Precautionary receive buffer garbage flush for 50ms
                clearTime = millis() + 50;
                while (millis() < clearTime && Serial.read() >= 0)
                    delay(0); // Equivalent to yield() for ESP8266;

                wait = false;           // No need to wait anymore
                lastCmdTime = millis(); // Set last received command time

                // Send screen size etc using a simple header with delimiters for client checks
                sendParameters(filename);
            }
        }
        else
        {
            // Check for time-out
            if (millis() > lastCmdTime + START_TIMEOUT)
                return false;
        }
    }

    uint8_t color[3 * NPIXELS]; // RGB and 565 format color buffer for N pixels

    // Send all the pixels on the whole screen
    for (int16_t y = 0; y < tft->height(); y++)
    {
        // Increment x by NPIXELS as we send NPIXELS for every byte received
        for (int16_t x = 0; x < tft->width(); x += NPIXELS)
        {
            delay(0); // Equivalent to yield() for ESP8266;

            // Wait here for serial data to arrive or a time-out elapses
            while (Serial.available() == 0)
            {
                if (millis() > lastCmdTime + PIXEL_TIMEOUT)
                    return false;
                delay(0); // Equivalent to yield() for ESP8266;
            }

            // Serial data must be available to get here, read 1 byte and
            // respond with N pixels, i.e. N x 3 RGB bytes or N x 2 565 format bytes
            if (Serial.read() == 'X')
            {
                // X command byte means abort, so clear the buffer and return
                clearTime = millis() + 50;
                while (millis() < clearTime && Serial.read() >= 0)
                    delay(0); // Equivalent to yield() for ESP8266;
                return false;
            }
            // Save arrival time of the read command (for later time-out check)
            lastCmdTime = millis();

#if defined BITS_PER_PIXEL && BITS_PER_PIXEL >= 24
            // Fetch N RGB pixels from x,y and put in buffer
            tft.readRectRGB(x, y, NPIXELS, 1, color);
            // Send buffer to client
            Serial.write(color, 3 * NPIXELS); // Write all pixels in the buffer
#else
            // Fetch N 565 format pixels from x,y and put in buffer
            tft->readRect(x, y, NPIXELS, 1, (uint16_t *)color);
            // Send buffer to client
            Serial.write(color, 2 * NPIXELS); // Write all pixels in the buffer
#endif
        }
    }

    Serial.flush(); // Make sure all pixel bytes have been despatched

    return true;
}

//====================================================================================
//    Send screen size etc using a simple header with delimiters for client checks
//====================================================================================
void DisplayTFT::sendParameters(String filename)
{
    Serial.write('W'); // Width
    Serial.write(tft->width() >> 8);
    Serial.write(tft->width() & 0xFF);

    Serial.write('H'); // Height
    Serial.write(tft->height() >> 8);
    Serial.write(tft->height() & 0xFF);

    Serial.write('Y'); // Bits per pixel (16 or 24)
    Serial.write(BITS_PER_PIXEL);

    Serial.write('?'); // Filename next
    Serial.print(filename);

    Serial.write('.'); // End of filename marker

    Serial.write(FILE_EXT); // Filename extension identifier

    Serial.write(*FILE_TYPE); // First character defines file type j,b,p,t
}

/****************************************************************************************
 * 
 *  From TFT library samples
 * 
****************************************************************************************/

// #########################################################################
// Draw a circular or elliptical arc with a defined thickness
// #########################################################################

// x,y == coords of centre of arc
// start_angle = 0 - 359
// seg_count = number of 6 degree segments to draw (60 => 360 degree arc)
// rx = x axis outer radius
// ry = y axis outer radius
// w  = width (thickness) of arc in pixels
// colour = 16 bit colour value
// Note if rx and ry are the same then an arc of a circle is drawn

int DisplayTFT::fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour)
{
    byte seg = TEMP_ARC_DEGREE_PER_SEG; // Segments are 3 degrees wide = 120 segments for 360 degrees
    byte inc = TEMP_ARC_DEGREE_PER_SEG; // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Calculate first pair of coordinates for segment start
    float sx = cos((start_angle - 90) * DEG2RAD);
    float sy = sin((start_angle - 90) * DEG2RAD);
    uint16_t x0 = sx * (rx - w) + x;
    uint16_t y0 = sy * (ry - w) + y;
    uint16_t x1 = sx * rx + x;
    uint16_t y1 = sy * ry + y;

    // Draw colour blocks every inc degrees
    int i;
    for (i = start_angle; i < start_angle + seg * seg_count; i += inc)
    {

        // Calculate pair of coordinates for segment end
        float sx2 = cos((i + seg - 90) * DEG2RAD);
        float sy2 = sin((i + seg - 90) * DEG2RAD);
        int x2 = sx2 * (rx - w) + x;
        int y2 = sy2 * (ry - w) + y;
        int x3 = sx2 * rx + x;
        int y3 = sy2 * ry + y;

        tft->fillTriangle(x0, y0, x1, y1, x2, y2, colour);
        tft->fillTriangle(x1, y1, x2, y2, x3, y3, colour);

        // Copy segment end to sgement start for next segment
        x0 = x2;
        y0 = y2;
        x1 = x3;
        y1 = y3;
    }

    return i;
}