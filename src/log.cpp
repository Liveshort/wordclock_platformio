#include <Arduino.h>

#include "log.h"
#include "main.h"

#define LOG_MAX_LENGTH 20

Logger::Logger() {
    _serial_history_idx = 0;
    _serial_history_length = 0;
}

void Logger::print(const String str) {
    _serial_history[_serial_history_idx] += str;

    Serial.print(str);
}

void Logger::println(const String str) {
    _serial_history[_serial_history_idx] += str;

    // Get the timestamp. If the time has not been set yet, use the current time in seconds since the start of the
    // program.
    if (!FLAGS[TIME_INITIALIZED])
        _serial_history_timestamps[_serial_history_idx] = "START+" + String(millis() / 1000);
    else
        _serial_history_timestamps[_serial_history_idx] = STRINGS[TIMESTAMP];

    // Update the index and length of the history. Overwrite the 101th element if the length is 100.
    _serial_history_idx = (_serial_history_idx + 1) % (LOG_MAX_LENGTH + 1);
    if (_serial_history_length < LOG_MAX_LENGTH)
        _serial_history_length++;
    _serial_history_timestamps[_serial_history_idx] = "";
    _serial_history[_serial_history_idx] = "";

    Serial.println(str);
}

String Logger::get_line_i_timestamp_from_serial_history(int i) {
    if (i > LOG_MAX_LENGTH)
        return "";
    if (_serial_history_length < LOG_MAX_LENGTH && _serial_history_idx - (i + 1) < 0)
        return "";

    int index = (_serial_history_idx - (i + 1)) % (LOG_MAX_LENGTH + 1);
    if (index < 0)
        index += (LOG_MAX_LENGTH + 1);

    return _serial_history_timestamps[index];
}

String Logger::get_line_i_content_from_serial_history(int i) {
    if (i > LOG_MAX_LENGTH)
        return "";
    if (_serial_history_length < LOG_MAX_LENGTH && _serial_history_idx - (i + 1) < 0)
        return "";

    int index = (_serial_history_idx - (i + 1)) % (LOG_MAX_LENGTH + 1);
    if (index < 0)
        index += (LOG_MAX_LENGTH + 1);
    String res = _serial_history[index];

    return res;
}
