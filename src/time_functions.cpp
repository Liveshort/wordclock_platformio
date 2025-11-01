#include <time.h>
#include <esp_sntp.h>

#include "main.h"
#include "log.h"
#include "time_functions.h"
#include "led_control.h"

// Default timezone is Amsterdam
// Other timezones can be found on https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* default_timezone = "CET-1CEST,M3.5.0,M10.5.0/3";


void time_synchronized_cb(struct timeval *tv) {
    LOGGER.println("Tijd gesynchroniseerd met NTP server.");
    TIMERS[TIME_SYNC] = millis()/1000;
}


void initialize_sntp_time_servers() {
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
    // First check if time is initialized
    if (!FLAGS[TIME_INITIALIZED]) FLAGS[TIME_INITIALIZED] = check_time_synchronization();
    // Only update if at least 500 ms have passed since last update
    if (!FLAGS[TIME_INITIALIZED] || millis() - TIMERS[TIME_UPDATE] < 500) return false;

    TIMERS[TIME_UPDATE] = millis();

    struct tm timeinfo;

    if(!getLocalTime(&timeinfo, 5)){
        return false;
    }

    strftime(STRINGS[TARGET_TIME], 10, "%H:%M:%S", &timeinfo);
    strftime(STRINGS[TIMESTAMP], 20, "%d-%b %H:%M:%S", &timeinfo);
    strftime(STRINGS[TIME_ZONE], 20, "%Z (%z)", &timeinfo);

    Serial.print("year:");
    Serial.print(timeinfo.tm_year + 1900); // years since 1900
    Serial.print("\tmonth:");
    Serial.print(timeinfo.tm_mon + 1); // January = 0 (!)
    Serial.print("\tday:");
    Serial.print(timeinfo.tm_mday); // day of month
    Serial.print("\thour:");
    Serial.print(timeinfo.tm_hour); // hours since midnight  0-23
    Serial.print("\tmin:");
    Serial.print(timeinfo.tm_min); // minutes after the hour  0-59
    Serial.print("\tsec:");
    Serial.println(timeinfo.tm_sec); // seconds after the minute  0-61*

    // ROUND_DOWN_TIME determines how to round the minutes to the nearest 5
    // If true, 12:29:59 -> 12:25 and 12:34:59 -> 12:30
    // If false, 12:27:30 -> 12:30 and 12:32:29 -> 12:30
    int rounded_hour, rounded_min;
    if (FLAGS[ROUND_DOWN_TIME]) {
        rounded_min = timeinfo.tm_min / 5 * 5;
    } else {
        if (timeinfo.tm_sec >= 30) rounded_min = (timeinfo.tm_min + 3) / 5 * 5;
        else rounded_min = (timeinfo.tm_min + 2) / 5 * 5;
    }
    MINUTE_DOTS = timeinfo.tm_min % 5;
    rounded_hour = timeinfo.tm_hour;
    // For 20+ minutes, round up the hour (e.g. 12:20 -> tien voor half een)
    if (rounded_min >= 20) rounded_hour = (timeinfo.tm_hour + 1) % 12;
    else rounded_hour = timeinfo.tm_hour % 12;
    // "0 uur" should be represented as "12 uur"
    if (rounded_hour == 0) rounded_hour = 12;
    // Handle the edge case of rounding 58 or 59 minutes up to the next hour
    if (rounded_min == 60) rounded_min = 0;
    
    for (int i = 0; i < 7; ++i) TARGET_TIME_WORDS[i] = 255;

    // HET IS VIJF VOOR HALF TWEE UUR
    // Set the words based on the rounded time
    switch (rounded_min) {
        case 0:
            TARGET_TIME_WORDS[0] = HET;
            TARGET_TIME_WORDS[1] = IS;
            TARGET_TIME_WORDS[6] = UUR;
            break;
        case 5:
            TARGET_TIME_WORDS[2] = VIJF;
            TARGET_TIME_WORDS[3] = OVER;
            break;
        case 10:
            TARGET_TIME_WORDS[2] = TIEN;
            TARGET_TIME_WORDS[3] = OVER;
            break;
        case 15:
            TARGET_TIME_WORDS[2] = KWART;
            TARGET_TIME_WORDS[3] = OVER;
            break;
        case 20:
            TARGET_TIME_WORDS[2] = TIEN;
            TARGET_TIME_WORDS[3] = VOOR;
            TARGET_TIME_WORDS[4] = HALF;
            break;
        case 25:
            TARGET_TIME_WORDS[2] = VIJF;
            TARGET_TIME_WORDS[3] = VOOR;
            TARGET_TIME_WORDS[4] = HALF;
            break;
        case 30:
            TARGET_TIME_WORDS[0] = HET;
            TARGET_TIME_WORDS[1] = IS;
            TARGET_TIME_WORDS[4] = HALF;
            break;
        case 35:
            TARGET_TIME_WORDS[2] = VIJF;
            TARGET_TIME_WORDS[3] = OVER;
            TARGET_TIME_WORDS[4] = HALF;
            break;
        case 40:
            TARGET_TIME_WORDS[2] = TIEN;
            TARGET_TIME_WORDS[3] = OVER;
            TARGET_TIME_WORDS[4] = HALF;
            break;
        case 45:
            TARGET_TIME_WORDS[2] = KWART;
            TARGET_TIME_WORDS[3] = VOOR;
            break;
        case 50:
            TARGET_TIME_WORDS[2] = TIEN;
            TARGET_TIME_WORDS[3] = VOOR;
            break;
        case 55:
            TARGET_TIME_WORDS[2] = VIJF;
            TARGET_TIME_WORDS[3] = VOOR;
            break;
        default:
            LOGGER.println("Onverwachte minuutwaarde bij tijd afronden: " + String(rounded_min));
            break;
    }

    // Set the hour word
    switch (rounded_hour) {
        case 1:
            TARGET_TIME_WORDS[5] = EEN2;
            break;
        case 2:
            TARGET_TIME_WORDS[5] = TWEE;
            break;
        case 3:
            TARGET_TIME_WORDS[5] = DRIE;
            break;
        case 4:
            TARGET_TIME_WORDS[5] = VIER;
            break;
        case 5:
            TARGET_TIME_WORDS[5] = VIJF2;
            break;
        case 6:
            TARGET_TIME_WORDS[5] = ZES;
            break;
        case 7:
            TARGET_TIME_WORDS[5] = ZEVEN;
            break;
        case 8:
            TARGET_TIME_WORDS[5] = ACHT;
            break;
        case 9:
            TARGET_TIME_WORDS[5] = NEGEN;
            break;
        case 10:
            TARGET_TIME_WORDS[5] = TIEN2;
            break;
        case 11:
            TARGET_TIME_WORDS[5] = ELF;
            break;
        case 12:
            TARGET_TIME_WORDS[5] = TWAALF;
            break;
        default:
            LOGGER.println("Onverwachte uurwaarde bij tijd afronden: " + String(rounded_hour));
            break;
    }

    for (int i = 0; i < 7; ++i) {
        if (TARGET_TIME_WORDS[i] == 255) continue;

        Serial.print(WORD_STRINGS[TARGET_TIME_WORDS[i]]);
        Serial.print(" ");
    }
    Serial.println();

    return true;
}

void set_current_time_to_target_time() {
    strncpy(STRINGS[CURRENT_TIME], STRINGS[TARGET_TIME], 10);
    for (int i = 0; i < 7; ++i) {
        CURRENT_TIME_WORDS[i] = TARGET_TIME_WORDS[i];
    }
}