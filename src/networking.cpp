#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <esp_wifi.h>

#include "main.h"
#include "networking.h"
#include "html_pages.h"
#include "persistent_storage.h"
#include "log.h"

#define WIFI_SCAN_INTERVAL 10000 // milliseconds
#define WIFI_STATUS_UPDATE_INTERVAL 1000 // milliseconds
#define WIFI_MAX_CONNECT_ATTEMPTS 30
#define WIFI_CONNECT_ATTEMPT_INTERVAL 1000 // milliseconds

const char* wifi_status_string[] = {"Verbinding maken...", "Geen SSID's beschikbaar", "Netwerkscan klaar", "Verbonden", "Verbinden mislukt", "Verbinding verbroken", "Niet verbonden"};

const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const String APIP_URL = "http://4.3.2.1";

const char* AP_ssid = "WoordKlok-0001";
const char* AP_password = "woordklok";

const char* http_user = "admin";
const char* http_pass = "1234";

const byte dns_port = 53;

WCNetworkManager::WCNetworkManager() {
    wifi_connect_attempt_counter = 0;
    wifi_unable_to_connect_counter = 0;

    wifi_status = 0;
    wifi_ssid = "-";
    wifi_password = "";
    wifi_last_connected = 0;
    
    http_login_enabled = false;
}

AsyncWebServer server(80);
DNSServer dnsServer;

class CaptiveRequestHandler : public AsyncWebHandler {
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request){
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        if (!request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", index_html); 
    }
};


void WCNetworkManager::setup_server() {
    // Required
	server.on("/connecttest.txt", [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	server.on("/wpad.dat", [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

	// A Tier (commonly used by modern systems)
	server.on("/generate_204", [](AsyncWebServerRequest *request) { request->redirect(APIP_URL); });		   // android captive portal redirect
	server.on("/redirect", [](AsyncWebServerRequest *request) { request->redirect(APIP_URL); });			   // microsoft redirect
	server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) { request->redirect(APIP_URL); });  // apple call home
	server.on("/canonical.html", [](AsyncWebServerRequest *request) { request->redirect(APIP_URL); });	   // firefox captive portal call home
	server.on("/success.txt", [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	server.on("/ncsi.txt", [](AsyncWebServerRequest *request) { request->redirect(APIP_URL); });			   // windows call home

	// return 404 to webpage icon
	server.on("/favicon.ico", [](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon

    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", index_html);
    });
    
    server.on("/wifi_settings", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", wifi_html);
    });
    
    server.on("/log_page", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", log_html);
    });
    
    server.on("/drawing_board", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", drawing_board_html);
        // Enter drawing board mode
        if (CURR_STATE != DRAWING_BOARD) {
            NEXT_STATE = DRAWING_BOARD;
            FLAGS[TRIGGER_STATE_CHANGE] = true;
            TIMERS[DRAWING_BOARD_TIMER] = millis();
            LOGGER.println("Entering drawing board mode");
        }
    });
    
    server.on("/get_leds", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        
        // Build JSON response with LED states
        String json = "{\"leds\":[";
        for (int i = 0; i < 174; i++) {
            json += String(DRAWING_BOARD_LEDS[i]);
            if (i < 173) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });
    
    server.on("/set_led", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        
        if (request->hasParam("led", true) && request->hasParam("state", true)) {
            int led = request->getParam("led", true)->value().toInt();
            int state = request->getParam("state", true)->value().toInt();
            
            if (led >= 0 && led < 174 && (state == 0 || state == 1)) {
                DRAWING_BOARD_LEDS[led] = state;
                
                // Get RGB color values if provided
                if (request->hasParam("r", true) && request->hasParam("g", true) && request->hasParam("b", true)) {
                    DRAWING_BOARD_COLORS[led][0] = request->getParam("r", true)->value().toInt();
                    DRAWING_BOARD_COLORS[led][1] = request->getParam("g", true)->value().toInt();
                    DRAWING_BOARD_COLORS[led][2] = request->getParam("b", true)->value().toInt();
                }
                
                TIMERS[DRAWING_BOARD_TIMER] = millis(); // Reset timeout
                
                // Ensure we're in drawing board mode
                if (CURR_STATE != DRAWING_BOARD) {
                    NEXT_STATE = DRAWING_BOARD;
                    FLAGS[TRIGGER_STATE_CHANGE] = true;
                }
                
                request->send(200, "text/plain", "OK");
            } else {
                request->send(400, "text/plain", "Invalid parameters");
            }
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });
    
    server.on("/clear_leds", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        
        // Clear all LEDs
        for (int i = 0; i < 174; i++) {
            DRAWING_BOARD_LEDS[i] = 0;
        }
        TIMERS[DRAWING_BOARD_TIMER] = millis(); // Reset timeout
        LOGGER.println("Drawing board cleared");
        request->send(200, "text/plain", "Cleared");
    });
    
    server.on("/exit_drawing_board", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        
        // Return to normal operation
        if (CURR_STATE == DRAWING_BOARD) {
            NEXT_STATE = NORMAL_OPERATION;
            FLAGS[TRIGGER_STATE_CHANGE] = true;
            LOGGER.println("Exiting drawing board mode");
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/scan_wifi_networks", HTTP_GET, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send(200, "text/plain", "WiFi scan started");
        scan_wifi();
    });

    server.on("/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        String last_connected = "";
        
        if (wifi_status == 3) {
            last_connected = "Nu";
        } else if (TIMERS[WIFI_CONNECTED_T] == 0) {
            last_connected += "-";
        } else {
            unsigned long time_since_last_wifi_connection = millis()/1000 - TIMERS[WIFI_CONNECTED_T];
            unsigned int seconds = time_since_last_wifi_connection % 60;
            unsigned int minutes = (time_since_last_wifi_connection / 60) % 60;
            unsigned int hours = (time_since_last_wifi_connection / 3600) % 24;
            unsigned int days = (time_since_last_wifi_connection / 3600 / 24) % 365;
            unsigned int years = time_since_last_wifi_connection / 3600 / 24 / 365;

            if (years > 0) last_connected += String(years) + " j ";
            if (days > 0) last_connected += String(days) + " d ";
            if (hours > 0) last_connected += String(hours) + " h ";
            if (minutes > 0) last_connected += String(minutes) + " m ";
            last_connected += String(seconds) + " s geleden";
        }

        String last_time_sync_string = "";
        if (!FLAGS[TIME_INITIALIZED]) {
            last_time_sync_string = "-";
        } else {
            unsigned long time_since_last_time_sync = millis()/1000 - TIMERS[TIME_SYNC];
            unsigned int seconds = time_since_last_time_sync % 60;
            unsigned int minutes = (time_since_last_time_sync / 60) % 60;
            unsigned int hours = (time_since_last_time_sync / 3600) % 24;
            unsigned int days = (time_since_last_time_sync / 3600 / 24) % 365;
            unsigned int years = time_since_last_time_sync / 3600 / 24 / 365;

            if (years > 0) last_time_sync_string += String(years) + " j ";
            if (days > 0) last_time_sync_string += String(days) + " d ";
            if (hours > 0) last_time_sync_string += String(hours) + " h ";
            if (minutes > 0) last_time_sync_string += String(minutes) + " m ";
            last_time_sync_string += String(seconds) + " s geleden";
        }

        String json = "{\"status\":\"" + String(wifi_status_string[wifi_status]) +
                     "\",\"ssid\":\"" + wifi_ssid +
                     "\",\"last_connected\":\"" + last_connected + 
                     "\",\"time\":\"" + STRINGS[TARGET_TIME] + 
                     "\",\"timezone\":\"" + STRINGS[TIME_ZONE] + 
                     "\",\"time_last_updated\":\"" + last_time_sync_string + "\"}";

        request->send(200, "application/json", json);
    });

    server.on("/log_short", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{\"timestamps\": [";
        String curr_line, next_line;
        for (int i = 0; i < 10; i++) {
            if (i == 0) {
                curr_line = LOGGER.get_line_i_timestamp_from_serial_history(i);
                next_line = LOGGER.get_line_i_timestamp_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = LOGGER.get_line_i_timestamp_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;
            
            if (i < 9) json += ",";
        }
        json += "], \"contents\": [";
        for (int i = 0; i < 10; i++) {
            if (i == 0) {
                curr_line = LOGGER.get_line_i_content_from_serial_history(i);
                next_line = LOGGER.get_line_i_content_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = LOGGER.get_line_i_content_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;

            if (i < 9) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });
    
    server.on("/log_long", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "{\"timestamps\": [";
        String curr_line, next_line;
        for (int i = 0; i < LOG_MAX_LENGTH; i++) {
            if (i == 0) {
                curr_line = LOGGER.get_line_i_timestamp_from_serial_history(i);
                next_line = LOGGER.get_line_i_timestamp_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = LOGGER.get_line_i_timestamp_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;
            
            if (i < LOG_MAX_LENGTH - 1) json += ",";
        }
        json += "], \"contents\": [";
        for (int i = 0; i < LOG_MAX_LENGTH; i++) {
            if (i == 0) {
                curr_line = LOGGER.get_line_i_content_from_serial_history(i);
                next_line = LOGGER.get_line_i_content_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = LOGGER.get_line_i_content_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;

            if (i < LOG_MAX_LENGTH - 1) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });

    server.on("/ssids", HTTP_GET, [this](AsyncWebServerRequest *request){
        String json = "[";
        for (int i = 0; i < ssid_count; i++) {
            json += "\"" + String(ssid_list[i]) + "\"";
            if (i < ssid_count - 1) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });
    
    server.on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            wifi_ssid = request->getParam("ssid", true)->value();
            wifi_password = request->getParam("password", true)->value();
            LOGGER.println("Inloggegevens ontvangen voor netwerk " + wifi_ssid + ".");
            connect_to_new_wifi_from_AP();
        }
        request->send(200, "text/plain", "Received");
    });

    server.on("/checkbox", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        if (request->hasParam("theme", true) && request->hasParam("checked", true)) {
            String theme = request->getParam("theme", true)->value();
            String checked = request->getParam("checked", true)->value();
            LOGGER.println("Theme " + theme + " is " + (checked == "1" ? "checked" : "unchecked"));
        }
        request->send(200, "text/plain", "Checkbox received");
    });
    
    server.on("/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        if (request->hasParam("action", true)) {
            String action = request->getParam("action", true)->value();
            LOGGER.println("Calibration action: " + action);
        }
        request->send(200, "text/plain", "Calibration received");
    });

    // the catch all
	server.onNotFound([](AsyncWebServerRequest *request) {
		request->redirect(APIP_URL);
		LOGGER.println("WebServer: Pagina niet gevonden (" + request->host() + " " + request->url() + ").  Doorgestuurd naar " + APIP_URL + ".");
	});

    dnsServer.setTTL(3600);
    dnsServer.start(dns_port, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    
    server.begin();
}

void WCNetworkManager::turn_on_wifi() {
    FLAGS[WIFI_ACTIVE] = true;

    if (STORAGE.check_saved_wifi_credentials()) STORAGE.load_wifi_credentials(wifi_ssid, wifi_password);
    WiFi.mode(WIFI_STA);

    LOGGER.println("Verbinding maken met " + wifi_ssid + "...");
    FLAGS[WIFI_CONNECTING] = true;
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}

void WCNetworkManager::turn_on_wifi_and_AP() {
    FLAGS[WIFI_ACTIVE] = true;
    FLAGS[AP_ACTIVE] = true;

    // Empty the current list of SSIDs
    ssid_list[0] = {"Start eerst netwerkscan..."};
    ssid_count = 1;
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(AP_ssid, AP_password, 6, 0, 4);
    LOGGER.print("Access Point opgestart op IP ");
    LOGGER.println(WiFi.softAPIP().toString() + ".");

    // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();

    delay(100);

    setup_server();
    
    LOGGER.println("Server opgestart.");

    if (STORAGE.check_saved_wifi_credentials()) {
        STORAGE.load_wifi_credentials(wifi_ssid, wifi_password);
        LOGGER.println("Verbinding maken met " + wifi_ssid + "...");
        FLAGS[WIFI_CONNECTING] = true;
        WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
    }
}

void WCNetworkManager::update() {
    if (FLAGS[WIFI_ACTIVE] && !FLAGS[AP_ACTIVE]) {
        update_wifi();
    } else if (FLAGS[WIFI_ACTIVE] && FLAGS[AP_ACTIVE]) {
        update_wifi_and_AP();
    }
}

void WCNetworkManager::update_wifi() {
    // Update the WiFi status every second
    if (millis() - TIMERS[WIFI_STATUS_UPDATE] > WIFI_STATUS_UPDATE_INTERVAL) {
        wifi_status = WiFi.status();
        TIMERS[WIFI_STATUS_UPDATE] = millis();
        if (wifi_status == WL_CONNECTED) {
            TIMERS[WIFI_CONNECTED_T] = millis();
            FLAGS[WIFI_CONNECTED_F] = true;
        } else {
            FLAGS[WIFI_CONNECTED_F] = false;
        }
    }

    // If we are connecting to a network, keep track of wifi status and number of attempts
    // and reset the WiFi connection if it fails
    if (FLAGS[WIFI_CONNECTING] && millis() - TIMERS[WIFI_CONNECT_ATTEMPT] > WIFI_CONNECT_ATTEMPT_INTERVAL) {
        if (wifi_status == WL_CONNECTED) {
            // We are connected to the network
            FLAGS[WIFI_CONNECTING] = false;
            wifi_connect_attempt_counter = 0;
            wifi_unable_to_connect_counter = 0;
            TIMERS[WIFI_CONNECT_ATTEMPT] = millis();
            STORAGE.save_wifi_credentials(wifi_ssid, wifi_password);
            
            LOGGER.println("--- Verbonden met netwerk " + wifi_ssid + " (" + WiFi.localIP().toString() + ")");
        } else {
            if (wifi_connect_attempt_counter < WIFI_MAX_CONNECT_ATTEMPTS) {
                // Try to connect to the network again
                wifi_connect_attempt_counter++;
                TIMERS[WIFI_CONNECT_ATTEMPT] = millis();
            } else {
                // If we have tried to connect to the network too many times, give up and try again later
                FLAGS[WIFI_CONNECTING] = false;
                wifi_connect_attempt_counter = 0;
                LOGGER.println("--- Verbinding maken met netwerk " + wifi_ssid + " mislukt. Wachtwoord mogelijk incorrect.");
                
                // Turn WiFi off for now
                wifi_unable_to_connect_counter++;
                FLAGS[WIFI_ACTIVE] = false;
                WiFi.mode(WIFI_OFF);
                
                // Set the timer for last failed connection attempt
                TIMERS[WIFI_CONNECT_FAILED] = millis();
            }
        }
    }
}

void WCNetworkManager::update_wifi_and_AP() {
    dnsServer.processNextRequest();

    // Update the WiFi status every second
    if (millis() - TIMERS[WIFI_STATUS_UPDATE] > WIFI_STATUS_UPDATE_INTERVAL) {
        wifi_status = WiFi.status();
        TIMERS[WIFI_STATUS_UPDATE] = millis();
        if (wifi_status == WL_CONNECTED) {
            TIMERS[WIFI_CONNECTED_T] = millis();
            FLAGS[WIFI_CONNECTED_F] = true;
        } else {
            FLAGS[WIFI_CONNECTED_F] = false;
        }
    }

    // If we are connecting to a network, keep track of wifi status and number of attempts
    // and reset the WiFi connection if it fails
    if (FLAGS[WIFI_CONNECTING] && millis() - TIMERS[WIFI_CONNECT_ATTEMPT] > WIFI_CONNECT_ATTEMPT_INTERVAL) {
        if (wifi_status == WL_CONNECTED) {
            // We are connected to the network
            FLAGS[WIFI_CONNECTING] = false;
            wifi_connect_attempt_counter = 0;
            TIMERS[WIFI_CONNECT_ATTEMPT] = millis();
            STORAGE.save_wifi_credentials(wifi_ssid, wifi_password);

            LOGGER.println("--- Verbonden met netwerk " + wifi_ssid + " (" + WiFi.localIP().toString() + ")");
        } else {
            if (wifi_connect_attempt_counter < WIFI_MAX_CONNECT_ATTEMPTS) {
                // Try to connect to the network again
                wifi_connect_attempt_counter++;
                TIMERS[WIFI_CONNECT_ATTEMPT] = millis();
            } else {
                // If we have tried to connect to the network too many times, give up and try again later
                FLAGS[WIFI_CONNECTING] = false;
                wifi_connect_attempt_counter = 0;
                LOGGER.println("--- Verbinding maken met netwerk " + wifi_ssid + " mislukt. Wachtwoord mogelijk incorrect.");
                
                // Set the timer for last failed connection attempt
                TIMERS[WIFI_CONNECT_FAILED] = millis();
            }
        }
    }
}

void WCNetworkManager::connect_to_new_wifi_from_AP() {
    LOGGER.println("Verbinding maken met netwerk " + wifi_ssid + "...");
    FLAGS[WIFI_CONNECTING] = true;
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}

void WCNetworkManager::scan_wifi() {
    if (millis() - TIMERS[WIFI_SCAN] < WIFI_SCAN_INTERVAL) {
        LOGGER.println("Er is al een netwerkscan bezig, probeer het later opnieuw.");
        return;
    }

    TIMERS[WIFI_SCAN] = millis();
    LOGGER.println("Netwerkscan starten...");
    
    // Save current state so we can restore it after the scan
    bool was_connecting = FLAGS[WIFI_CONNECTING];
    bool was_connected = (WiFi.status() == WL_CONNECTED);
    String saved_ssid = wifi_ssid;
    String saved_password = wifi_password;
    
    // Always disconnect before scanning - scan has priority
    LOGGER.println("WiFi disconnecten voor netwerkscan...");
    FLAGS[WIFI_CONNECTING] = false;
    esp_wifi_disconnect();
    delay(200);
  
    // Perform the scan
    int n = WiFi.scanNetworks();
    
    // Handle scan results
    if (n == -2) {
        LOGGER.println("--- Netwerkscan mislukt (foutcode -2 WIFI_SCAN_FAILED).");
        ssid_list[0] = "Netwerkscan mislukt...";
        ssid_count = 1;
    } else if (n == 0) {
        LOGGER.println("--- Geen netwerken gevonden.");
        ssid_list[0] = "Geen netwerken gevonden...";
        ssid_count = 1;
    } else if (n > 0) {
        LOGGER.print("--- ");
        LOGGER.print(String(n));
        LOGGER.println(" netwerken gevonden.");
  
        if (n > 50) n = 50;
  
        for (int i = 0; i < n; ++i) {
            ssid_list[i] = WiFi.SSID(i).c_str();
        }
        ssid_count = n;
    }
    
    // Restore connection if we were connected or connecting before
    if ((was_connecting || was_connected) && !saved_ssid.isEmpty()) {
        LOGGER.println("Hervatten verbinding met " + saved_ssid + "...");
        wifi_ssid = saved_ssid;
        wifi_password = saved_password;
        FLAGS[WIFI_CONNECTING] = true;
        WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
        wifi_connect_attempt_counter = 0;
        TIMERS[WIFI_CONNECT_ATTEMPT] = millis();
    }
  
//    LOGGER.println("Nr | SSID                             | RSSI | CH | Encryption");
//    for (int i = 0; i < n; ++i) {
//      // LOGGER.print SSID and RSSI for each network found
//      Serial.printf("%2d", i + 1);
//      LOGGER.print(" | ");
//      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
//      LOGGER.print(" | ");
//      Serial.printf("%4ld", WiFi.RSSI(i));
//      LOGGER.print(" | ");
//      Serial.printf("%2ld", WiFi.channel(i));
//      LOGGER.print(" | ");
//      switch (WiFi.encryptionType(i)) {
//        case WIFI_AUTH_OPEN:            LOGGER.print("open"); break;
//        case WIFI_AUTH_WEP:             LOGGER.print("WEP"); break;
//        case WIFI_AUTH_WPA_PSK:         LOGGER.print("WPA"); break;
//        case WIFI_AUTH_WPA2_PSK:        LOGGER.print("WPA2"); break;
//        case WIFI_AUTH_WPA_WPA2_PSK:    LOGGER.print("WPA+WPA2"); break;
//        case WIFI_AUTH_WPA2_ENTERPRISE: LOGGER.print("WPA2-EAP"); break;
//        case WIFI_AUTH_WPA3_PSK:        LOGGER.print("WPA3"); break;
//        case WIFI_AUTH_WPA2_WPA3_PSK:   LOGGER.print("WPA2+WPA3"); break;
//        case WIFI_AUTH_WAPI_PSK:        LOGGER.print("WAPI"); break;
//        default:                        LOGGER.print("unknown");
//      }
//      LOGGER.println();
//      delay(10);
//    }
//  }
//  LOGGER.println("");

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
}
