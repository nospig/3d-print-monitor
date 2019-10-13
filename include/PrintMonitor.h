#ifndef _print_monitor_h
#define _print_monitor_h

#include "DisplayBase.h"

// task callbacks
void connectWifiCallback();
void getTimeCallback();
void getCurrentWeatherCallback();
void updateWifiStrengthCallback();
void checkSettingsChangedCallback();
void checkScreenGrabCallback();
void updatePrinterMonitorCallback();

void setupOtaUpdates();

#endif // _print_monitor_h