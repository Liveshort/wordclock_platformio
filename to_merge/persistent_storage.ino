bool check_saved_wifi_credentials() {
    if (prefs.isKey("wifi_ssid") && prefs.isKey("wifi_password")) return true;
    
    return false;
}

void save_wifi_credentials(String ssid, String password) {
    prefs.putString("wifi_ssid", ssid);
    prefs.putString("wifi_password", password);
}

void load_wifi_credentials(String &ssid, String &password) {
    ssid = prefs.getString("wifi_ssid");
    password = prefs.getString("wifi_password");
}