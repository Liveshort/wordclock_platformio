#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <esp_wifi.h>

unsigned int wifi_scan_interval = 10000;
unsigned long last_wifi_scan_millis = 0;
unsigned int wifi_status_update_interval = 1000;
unsigned long time_since_last_wifi_connection = 3122064000;
unsigned long last_wifi_status_update_millis = 0;
bool wifi_connecting_to_network = false;
unsigned int wifi_connect_attempt_interval = 500;
unsigned int wifi_connect_attempts = 20;
unsigned int wifi_connect_attempt_counter = 0;
unsigned int last_wifi_connect_attempt_millis = 0;
unsigned int wifi_unable_to_connect_counter = 0;

int wifi_status = 0;
const char* wifi_status_string[] = {"Verbinding maken...", "Geen SSID's beschikbaar", "Netwerkscan klaar", "Verbonden", "Verbinden mislukt", "Verbinding verbroken", "Niet verbonden"};
String wifi_ssid = "-";
String wifi_password = "";
unsigned long wifi_last_connected = 0;

const IPAddress localIP(4, 3, 2, 1);
const IPAddress gatewayIP(4, 3, 2, 1);
const IPAddress subnetMask(255, 255, 255, 0);
const String APIP_URL = "http://4.3.2.1";

const char* AP_ssid = "WoordKlok-0001";
const char* AP_password = "woordklok";

bool http_login_enabled = false;
const char* http_user = "admin";
const char* http_pass = "1234";

AsyncWebServer server(80);

DNSServer dnsServer;
const byte dns_port = 53;

String ssid_list[50];
int ssid_count;


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


void setup_server() {
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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", index_html);
    });
    
    server.on("/wifi_settings", HTTP_GET, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", wifi_html);
    });
    
    server.on("/log_page", HTTP_GET, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send_P(200, "text/html", log_html);
    });

    server.on("/scan_wifi_networks", HTTP_GET, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        request->send(200, "text/plain", "WiFi scan started");
        scan_wifi();
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
        String last_connected = "";
        
        if (wifi_status == 3) {
            last_connected = "Nu";
        } else if (wifi_ssid == "-" || wifi_connecting_to_network) {
            last_connected += "-";
        } else {
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
        if (!time_initialized) {
            last_time_sync_string = "-";
        } else {
            unsigned int time_since_last_time_sync = millis()/1000 - last_time_sync;
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
                     "\",\"time\":\"" + current_time_string + 
                     "\",\"timezone\":\"" + current_time_zone_string + 
                     "\",\"time_last_updated\":\"" + last_time_sync_string + "\"}";

        request->send(200, "application/json", json);
    });

    server.on("/log_short", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{\"timestamps\": [";
        String curr_line, next_line;
        for (int i = 0; i < 10; i++) {
            if (i == 0) {
                curr_line = get_line_i_timestamp_from_serial_history(i);
                next_line = get_line_i_timestamp_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = get_line_i_timestamp_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;
            
            if (i < 9) json += ",";
        }
        json += "], \"contents\": [";
        for (int i = 0; i < 10; i++) {
            if (i == 0) {
                curr_line = get_line_i_content_from_serial_history(i);
                next_line = get_line_i_content_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = get_line_i_content_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;

            if (i < 9) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });
    
    server.on("/log_long", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{\"timestamps\": [";
        String curr_line, next_line;
        for (int i = 0; i < LOG_MAX_LENGTH; i++) {
            if (i == 0) {
                curr_line = get_line_i_timestamp_from_serial_history(i);
                next_line = get_line_i_timestamp_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = get_line_i_timestamp_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;
            
            if (i < LOG_MAX_LENGTH - 1) json += ",";
        }
        json += "], \"contents\": [";
        for (int i = 0; i < LOG_MAX_LENGTH; i++) {
            if (i == 0) {
                curr_line = get_line_i_content_from_serial_history(i);
                next_line = get_line_i_content_from_serial_history(i+1);
                if (curr_line == "" && next_line == "") break;
            }

            json += "\"" + curr_line + "\"";

            curr_line = next_line;
            next_line = get_line_i_content_from_serial_history(i+2);
            if (curr_line == "" && next_line == "") break;

            if (i < LOG_MAX_LENGTH - 1) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json);
    });

    server.on("/ssids", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "[";
        for (int i = 0; i < ssid_count; i++) {
            json += "\"" + String(ssid_list[i]) + "\"";
            if (i < ssid_count - 1) json += ",";
        }
        json += "]";
        request->send(200, "application/json", json);
    });
    
    server.on("/submit", HTTP_POST, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
            wifi_ssid = request->getParam("ssid", true)->value();
            wifi_password = request->getParam("password", true)->value();
            println("Inloggegevens ontvangen voor netwerk " + wifi_ssid + ".");
            connect_to_new_wifi_from_AP();
        }
        request->send(200, "text/plain", "Received");
    });

    server.on("/checkbox", HTTP_POST, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        if (request->hasParam("theme", true) && request->hasParam("checked", true)) {
            String theme = request->getParam("theme", true)->value();
            String checked = request->getParam("checked", true)->value();
            println("Theme " + theme + " is " + (checked == "1" ? "checked" : "unchecked"));
        }
        request->send(200, "text/plain", "Checkbox received");
    });
    
    server.on("/calibrate", HTTP_POST, [](AsyncWebServerRequest *request){
        if (http_login_enabled && !request->authenticate(http_user, http_pass))
            return request->requestAuthentication();
        if (request->hasParam("action", true)) {
            String action = request->getParam("action", true)->value();
            println("Calibration action: " + action);
        }
        request->send(200, "text/plain", "Calibration received");
    });

    // the catch all
	server.onNotFound([](AsyncWebServerRequest *request) {
		request->redirect(APIP_URL);
		println("WebServer: Pagina niet gevonden (" + request->host() + " " + request->url() + ").  Doorgestuurd naar " + APIP_URL + ".");
	});

    dnsServer.setTTL(3600);
    dnsServer.start(dns_port, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    
    server.begin();
}


void turn_on_wifi() {
    if (check_saved_wifi_credentials()) load_wifi_credentials(wifi_ssid, wifi_password);
    WiFi.mode(WIFI_STA);

    println("Verbinding maken met " + wifi_ssid + "...");
    wifi_connecting_to_network = true;
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}


void turn_on_wifi_and_AP() {
    // Empty the current list of SSIDs
    ssid_list[0] = {"Start eerst netwerkscan..."};
    ssid_count = 1;
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    WiFi.softAP(AP_ssid, AP_password, 6, 0, 4);
    print("Access Point opgestart op IP ");
    println(WiFi.softAPIP().toString() + ".");

    // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
	esp_wifi_stop();
	esp_wifi_deinit();
	wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
	my_config.ampdu_rx_enable = false;
	esp_wifi_init(&my_config);
	esp_wifi_start();

    delay(100);

    setup_server();
    
    println("Server opgestart.");
}


void update_wifi() {
    // Update the WiFi status every second
    if (millis() - last_wifi_status_update_millis > wifi_status_update_interval) {
        wifi_status = WiFi.status();
        last_wifi_status_update_millis = millis();
        if (wifi_status == WL_CONNECTED) {
            time_since_last_wifi_connection = 0;
        } else {
            time_since_last_wifi_connection += wifi_status_update_interval / 1000;
        }
    }

    // If we are connecting to a network, keep track of wifi status and number of attempts
    // and reset the WiFi connection if it fails
    if (wifi_connecting_to_network && millis() - last_wifi_connect_attempt_millis > wifi_connect_attempt_interval) {
        if (wifi_status == WL_CONNECTED) {
            // We are connected to the network
            wifi_connecting_to_network = false;
            wifi_connect_attempt_counter = 0;
            wifi_unable_to_connect_counter = 0;
            last_wifi_connect_attempt_millis = millis();
            save_wifi_credentials(wifi_ssid, wifi_password);
            
            println("--- Verbonden met netwerk " + wifi_ssid + " (" + WiFi.localIP().toString() + ")");
        } else {
            if (wifi_connect_attempt_counter < wifi_connect_attempts) {
                // Try to connect to the network again
                wifi_connect_attempt_counter++;
                last_wifi_connect_attempt_millis = millis();
            } else {
                // If we have tried to connect to the network too many times, give up and try again later
                wifi_connecting_to_network = false;
                wifi_connect_attempt_counter = 0;
                println("--- Verbinding maken met netwerk " + wifi_ssid + " mislukt. Wachtwoord mogelijk incorrect.");
                wifi_ssid = "-";
                
                // Turn WiFi off for now
                wifi_unable_to_connect_counter++;
                wifi_active = false;
                WiFi.mode(WIFI_OFF);
            }
        }
    }
}


void update_wifi_and_AP() {
    dnsServer.processNextRequest();

    // Update the WiFi status every second
    if (millis() - last_wifi_status_update_millis > wifi_status_update_interval) {
        wifi_status = WiFi.status();
        last_wifi_status_update_millis = millis();
        if (wifi_status == WL_CONNECTED) {
            time_since_last_wifi_connection = 0;
        } else {
            time_since_last_wifi_connection += wifi_status_update_interval / 1000;
        }
    }

    // If we are connecting to a network, keep track of wifi status and number of attempts
    // and reset the WiFi connection if it fails
    if (wifi_connecting_to_network && millis() - last_wifi_connect_attempt_millis > wifi_connect_attempt_interval) {
        if (wifi_status == WL_CONNECTED) {
            // We are connected to the network
            wifi_connecting_to_network = false;
            wifi_connect_attempt_counter = 0;
            last_wifi_connect_attempt_millis = millis();
            save_wifi_credentials(wifi_ssid, wifi_password);

            println("--- Verbonden met netwerk " + wifi_ssid + " (" + WiFi.localIP().toString() + ")");
        } else {
            if (wifi_connect_attempt_counter < wifi_connect_attempts) {
                // Try to connect to the network again
                wifi_connect_attempt_counter++;
                last_wifi_connect_attempt_millis = millis();
            } else {
                // If we have tried to connect to the network too many times, give up and try again later
                wifi_connecting_to_network = false;
                wifi_connect_attempt_counter = 0;
                println("--- Verbinding maken met netwerk " + wifi_ssid + " mislukt. Wachtwoord mogelijk incorrect.");
                wifi_ssid = "-";
                
                // Reset the WiFi connection
                wifi_active = false;
                AP_active = false;
                WiFi.mode(WIFI_OFF);

                turn_on_wifi_and_AP();
                wifi_active = true;
                AP_active = true;
            }
        }
    }
}


void connect_to_new_wifi_from_AP() {
    println("Verbinding maken met netwerk " + wifi_ssid + "...");
    wifi_connecting_to_network = true;
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
}


void scan_wifi() {
    if (millis() - last_wifi_scan_millis < wifi_scan_interval) {
        println("Er is al een netwerkscan bezig, probeer het later opnieuw.");
        return;
    }

    last_wifi_scan_millis = millis();
    println("Netwerkscan starten...");
  
    // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    if (n == -2) {
        println("--- Netwerkscan mislukt (foutcode -2 WIFI_SCAN_FAILED).");
        ssid_list[0] = "Netwerkscan mislukt...";
        ssid_count = 1;
    } else if (n == 0) {
        println("--- Geen netwerken gevonden.");
        ssid_list[0] = "Geen netwerken gevonden...";
        ssid_count = 1;
    } else {
        print("--- ");
        print(String(n));
        println(" netwerken gevonden.");
  
        if (n > 50) n = 50;
  
        for (int i = 0; i < n; ++i) {
            ssid_list[i] = WiFi.SSID(i).c_str();
        }
        ssid_count = n;

        // for (int i = 0; i < n; ++i) {
        //     println(ssid_list[i]);
        // }
        // println("");
    }
  
//    println("Nr | SSID                             | RSSI | CH | Encryption");
//    for (int i = 0; i < n; ++i) {
//      // Print SSID and RSSI for each network found
//      Serial.printf("%2d", i + 1);
//      print(" | ");
//      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
//      print(" | ");
//      Serial.printf("%4ld", WiFi.RSSI(i));
//      print(" | ");
//      Serial.printf("%2ld", WiFi.channel(i));
//      print(" | ");
//      switch (WiFi.encryptionType(i)) {
//        case WIFI_AUTH_OPEN:            print("open"); break;
//        case WIFI_AUTH_WEP:             print("WEP"); break;
//        case WIFI_AUTH_WPA_PSK:         print("WPA"); break;
//        case WIFI_AUTH_WPA2_PSK:        print("WPA2"); break;
//        case WIFI_AUTH_WPA_WPA2_PSK:    print("WPA+WPA2"); break;
//        case WIFI_AUTH_WPA2_ENTERPRISE: print("WPA2-EAP"); break;
//        case WIFI_AUTH_WPA3_PSK:        print("WPA3"); break;
//        case WIFI_AUTH_WPA2_WPA3_PSK:   print("WPA2+WPA3"); break;
//        case WIFI_AUTH_WAPI_PSK:        print("WAPI"); break;
//        default:                        print("unknown");
//      }
//      println();
//      delay(10);
//    }
//  }
//  println("");

    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
}
