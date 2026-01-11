#ifndef _PERSISTENT_STORAGE_H_
#define _PERSISTENT_STORAGE_H_

#include <Arduino.h>
#include <Preferences.h>

extern Preferences PREFS;

class Storage {
   public:
    void initialize();
    bool check_saved_wifi_credentials();
    bool check_user_settings_saved();
    void save_user_settings();
    void load_user_settings();
    void default_user_settings();
    void save_wifi_credentials(String ssid, String password);
    void load_wifi_credentials(String& ssid, String& password);
};

#endif
