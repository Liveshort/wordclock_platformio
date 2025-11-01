#ifndef _NETWORKING_H_
#define _NETWORKING_H_

class WCNetworkManager {
   private:
    unsigned int wifi_connect_attempt_counter;
    unsigned int wifi_unable_to_connect_counter;

    int wifi_status;
    String wifi_ssid;
    String wifi_password;
    unsigned long wifi_last_connected;

    bool http_login_enabled;

    String ssid_list[50];
    int ssid_count;

   public:
    WCNetworkManager();
    void setup_server();
    void turn_on_wifi();
    void turn_on_wifi_and_AP();
    void update();
    void update_wifi();
    void update_wifi_and_AP();
    void connect_to_new_wifi_from_AP();
    void scan_wifi();
};

#endif
