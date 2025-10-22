#include "time.h"
#include "esp_sntp.h"

// Default timezone is Amsterdam
// Other timezones can be found on https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* default_timezone = "CET-1CEST,M3.5.0,M10.5.0/3";


void time_synchronized_cb(struct timeval *tv) {
    println("Tijd gesynchroniseerd met NTP server.");
    last_time_sync = millis()/1000;
}


void init_sntp() {
    sntp_set_sync_interval(5*60*1000UL);
    sntp_set_time_sync_notification_cb(time_synchronized_cb);
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_setservername(2, "time.cloudflare.com");
    esp_sntp_init();

    setenv("TZ", default_timezone, 1);
    tzset();
}


bool check_time_synchronization() {
    if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) return true;
    
    return false;
}


bool update_time() {
    struct tm timeinfo;

    if(!getLocalTime(&timeinfo, 5)){
        return false;
    }

    strftime(current_time_string, 20, "%H:%M:%S", &timeinfo);
    strftime(current_timestamp_string, 20, "%j-%H:%M:%S", &timeinfo);
    strftime(current_time_zone_string, 20, "%Z (%z)", &timeinfo);

    return true;
}