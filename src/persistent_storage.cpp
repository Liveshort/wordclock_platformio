#include <Preferences.h>

#include "main.h"
#include "persistent_storage.h"

Preferences PREFS;

void Storage::initialize() {
    PREFS.begin("wordclock", false);
}

bool Storage::check_saved_wifi_credentials() {
    if (PREFS.isKey("wifi_ssid") && PREFS.isKey("wifi_password"))
        return true;

    return false;
}

void Storage::save_wifi_credentials(String ssid, String password) {
    PREFS.putString("wifi_ssid", ssid);
    PREFS.putString("wifi_password", password);
}

void Storage::load_wifi_credentials(String& ssid, String& password) {
    ssid = PREFS.getString("wifi_ssid");
    password = PREFS.getString("wifi_password");
}
