#ifndef _user_settings_h
#define _user_settings_h

#include <Arduino.h>

// OTA password
#define OTA_MD5_PASSWORD_HASH "5f4dcc3b5aa765d61d8327deb882cf99"


// TFT settings

// change depending if your screen is pins up or down, 0 is usually down, 2 for up
#define SCREEN_ROTATION 2

// Pin used for brightness control. Just connect LED pin on TFT to 3.3V if not required.
#define BRIGHTNESS_PIN          16   // D0

// maximum number of printers allowed
#define MAX_PRINTERS 10

#endif // _user_settings_h