/**The MIT License (MIT)
 
 Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#include <Arduino.h>

typedef struct OpenWeatherMapCurrentData
{
    String main;
    String description;
    String icon;
    String location;
    float temp;
    float tempMin;
    float tempMax;
    uint16_t pressure;
    uint8_t humidity;
    float windSpeed;
    float windDeg;
    uint32_t observationTime;
    uint32_t sunRise;
    uint32_t sunSet;
    uint32_t timeZone;

    // -1 means no data for rain and clouds
    int cloudPercentage;
    int rainOneHour;
    int rainThreeHour;

    bool validData;
} OpenWeatherMapCurrentData;


class OpenWeatherMapCurrent
{
    public:
        OpenWeatherMapCurrent();
        void update(String appId, String location);
        void updateById(String appId, String locationId);

        void setMetric(boolean metric) { this->metric = metric; }
        boolean isMetric() { return metric; }

        void setLanguage(String language) { this->language = language; }
        String getLanguage() { return language; }

        OpenWeatherMapCurrentData* getCurrentData();

    private:
        boolean metric = true;
        String language;
        OpenWeatherMapCurrentData data;

        void doUpdate(String url);
        String buildUrl(String appId, String locationParameter);
        void deserializeWeather(String json);
        String captaliseString(String input);
};
