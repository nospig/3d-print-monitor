#ifndef _print_monitor_h
#define _print_monitor_h

#include "DisplayBase.h"

// task callbacks
void connectWifiCallback();
void getTimeCallback();
void getCurrentWeatherCallback();
void updateWifiStrengthCallback();
void checkScreenGrabCallback();
void updatePrinterMonitorCallback();
void cycleDisplayCallback();

void setupOtaUpdates();
void setupDisplay();

void settingsChangedCallback();

#endif // _print_monitor_h