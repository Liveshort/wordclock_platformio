#include <Preferences.h>

unsigned long last = 0;
unsigned long last_time_sync = 0;
unsigned long last_time_update = 0;

bool time_initialized = false;
char current_time_string[10] = "--:--:--";
char current_timestamp_string[20] = "------ --:--:--";
char current_time_zone_string[20] = "-";

bool wifi_active = false;
bool AP_active = false;

volatile bool button22_pressed = false;
volatile bool button23_pressed = false;

Preferences prefs;


void setup() {
    Serial.begin(115200);
    prefs.begin("wc_prefs", false);

    setup_buttons();

    if (check_saved_wifi_credentials()) {
        turn_on_wifi();
        wifi_active = true;
    } else {
        turn_on_wifi_and_AP();
        wifi_active = true;
        AP_active = true;
    }

    init_sntp();
}


void loop() {
    if (button22_pressed) {
        Serial.println("Button 22 pressed!");
        button22_pressed = false;
    }

    if (button23_pressed) {
        Serial.println("Button 23 pressed!");
        button23_pressed = false;
    }

    int light_transistor_value = analogRead(4);
    int ldr_value = analogRead(2);

    Serial.println("Transistor: " + String(light_transistor_value) + "| LDR: " + String(ldr_value));

    if (wifi_active && AP_active) update_wifi_and_AP();
    if (wifi_active && !AP_active) update_wifi();

    if (time_initialized && millis() - last_time_update > 500) {
        last_time_update = millis();
        update_time();
    }

    if (millis() - last > 10000) {
        if (!time_initialized) time_initialized = check_time_synchronization();
        
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo, 5)){
            Serial.println("Nog geen tijd beschikbaar...");
        } else {
            Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z");
        }

        last = millis();
    }

    delay(500);
}