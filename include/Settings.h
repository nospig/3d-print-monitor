#ifndef _settings_h
#define _settings_h

#include <Arduino.h>
#include "UserSettings.h"


// task scheduler
#define _TASK_SLEEP_ON_IDLE_RUN

// time
#define SECONDS_MULT 1000
#define MINUTES_MULT 60 * SECONDS_MULT
#define HOURS_MULT 60 * SECONDS_MULT

#define TIME_FETCH_INTERVAL 5 * SECONDS_MULT

#define WIFI_STRENGTH_INTERVAL          10 * SECONDS_MULT
#define SETTINGS_CHANGED_INTERVAL       10 * SECONDS_MULT
#define SCREENGRAB_INTERVAL             10 * SECONDS_MULT
#define WIFI_CONNECTING_DELAY           2 * SECONDS_MULT


#endif // _settings_h