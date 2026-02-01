#include <Preferences.h>

#include "main.h"
#include "persistent_storage.h"

Preferences PREFS;

void Storage::initialize() {
    PREFS.begin("wordclock", false);
}

bool Storage::check_user_settings_saved() {
    if (PREFS.isKey("setting_rdt") && PREFS.isKey("setting_sen") && PREFS.isKey("setting_sis") &&
        PREFS.isKey("setting_sds") && PREFS.isKey("setting_fcs") && PREFS.isKey("setting_acp") &&
        PREFS.isKey("setting_pin") && PREFS.isKey("setting_pcy") && PREFS.isKey("setting_prs") &&
        PREFS.isKey("setting_mbr") && PREFS.isKey("setting_mbt"))
        return true;

    return false;
}

void Storage::save_user_settings() {
    PREFS.putInt("setting_rdt", USER_SETTINGS[ROUND_DOWN_TIME]);
    PREFS.putInt("setting_sen", USER_SETTINGS[SAYINGS_ENABLED]);
    PREFS.putInt("setting_sis", USER_SETTINGS[SAYING_INTERVAL_S]);
    PREFS.putInt("setting_sds", USER_SETTINGS[SAYING_DURATION_S]);
    PREFS.putInt("setting_fcs", USER_SETTINGS[FADE_CYCLE_S]);
    PREFS.putInt("setting_acp", USER_SETTINGS[ACTIVE_PALETTES]);
    PREFS.putInt("setting_pin", USER_SETTINGS[PALETTE_INTERVAL_S]);
    PREFS.putInt("setting_pcy", USER_SETTINGS[PALETTE_CYCLE_S]);
    PREFS.putInt("setting_prs", USER_SETTINGS[PALETTE_ROW_SPACING]);
    PREFS.putInt("setting_mbr", USER_SETTINGS[MANUAL_BRIGHTNESS]);
    PREFS.putInt("setting_mbt", USER_SETTINGS[MANUAL_BRIGHTNESS_TIMEOUT_S]);
}

void Storage::load_user_settings() {
    USER_SETTINGS[ROUND_DOWN_TIME] = PREFS.getInt("setting_rdt");
    USER_SETTINGS[SAYINGS_ENABLED] = PREFS.getInt("setting_sen");
    USER_SETTINGS[SAYING_INTERVAL_S] = PREFS.getInt("setting_sis");
    USER_SETTINGS[SAYING_DURATION_S] = PREFS.getInt("setting_sds");
    USER_SETTINGS[FADE_CYCLE_S] = PREFS.getInt("setting_fcs");
    USER_SETTINGS[ACTIVE_PALETTES] = PREFS.getInt("setting_acp");
    USER_SETTINGS[PALETTE_INTERVAL_S] = PREFS.getInt("setting_pin");
    USER_SETTINGS[PALETTE_CYCLE_S] = PREFS.getInt("setting_pcy");
    USER_SETTINGS[PALETTE_ROW_SPACING] = PREFS.getInt("setting_prs");
    USER_SETTINGS[MANUAL_BRIGHTNESS] = PREFS.getInt("setting_mbr");
    USER_SETTINGS[MANUAL_BRIGHTNESS_TIMEOUT_S] = PREFS.getInt("setting_mbt");

    // String log_line = "Loaded user settings:\n";
    // LOGGER.println(log_line);
    // String log_line = "  ROUND_DOWN_TIME: " + std::to_string(USER_SETTINGS[ROUND_DOWN_TIME]) + "\n";
    // LOGGER.print(log_line);

    LOGGER.print("Gebruikersinstellingen geladen: ");
    LOGGER.print(String(USER_SETTINGS[ROUND_DOWN_TIME]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[SAYINGS_ENABLED]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[SAYING_INTERVAL_S]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[SAYING_DURATION_S]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[FADE_CYCLE_S]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[ACTIVE_PALETTES]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[PALETTE_INTERVAL_S]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[PALETTE_CYCLE_S]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[PALETTE_ROW_SPACING]));
    LOGGER.print(", ");
    LOGGER.print(String(USER_SETTINGS[MANUAL_BRIGHTNESS]));
    LOGGER.print(", ");
    LOGGER.println(String(USER_SETTINGS[MANUAL_BRIGHTNESS_TIMEOUT_S]));
}

void Storage::default_user_settings() {
    PREFS.putInt("setting_rdt", 1);
    PREFS.putInt("setting_sen", 1);
    PREFS.putInt("setting_sis", 40);
    PREFS.putInt("setting_sds", 15);
    PREFS.putInt("setting_fcs", 4);
    PREFS.putInt("setting_acp", 511);
    PREFS.putInt("setting_pin", 120);
    PREFS.putInt("setting_pcy", 10);
    PREFS.putInt("setting_prs", 3);
    PREFS.putInt("setting_mbr", 255);
    PREFS.putInt("setting_mbt", 60);

    LOGGER.println("Geen instellingen gevonden, standaardinstellingen opgeslagen.");
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
