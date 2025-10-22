#ifndef _PERSISTENT_STORAGE_H_
#define _PERSISTENT_STORAGE_H_

#include <Arduino.h>
#include <Preferences.h>

extern Preferences PREFS;

class Storage {
    public:
    void initialize();
    bool check_saved_wifi_credentials();
    void save_wifi_credentials(String ssid, String password);
    void load_wifi_credentials(String &ssid, String &password);
};

#endif