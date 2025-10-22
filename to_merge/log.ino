#define LOG_MAX_LENGTH 20
String serial_history[LOG_MAX_LENGTH + 1];
String serial_history_timestamps[LOG_MAX_LENGTH + 1];
int serial_history_idx = 0;
int serial_history_length = 0;


void print(const String str) {
    serial_history[serial_history_idx] += str;

    Serial.print(str);
}


void println(const String str) {
    serial_history[serial_history_idx] += str;

    // Get the timestamp. If the time has not been set yet, use the current time in seconds since the start of the program.
    if (!time_initialized) serial_history_timestamps[serial_history_idx] = "START+" + String(millis()/1000);
    else serial_history_timestamps[serial_history_idx] = current_timestamp_string;

    // Update the index and length of the history. Overwrite the 101th element if the length is 100.
    serial_history_idx = (serial_history_idx + 1) % (LOG_MAX_LENGTH + 1);
    if (serial_history_length < LOG_MAX_LENGTH) serial_history_length++;
    serial_history_timestamps[serial_history_idx] = "";
    serial_history[serial_history_idx] = "";

    Serial.println(str);
}


String get_line_i_timestamp_from_serial_history(int i) {
    if (i > LOG_MAX_LENGTH) return "";
    if (serial_history_length < LOG_MAX_LENGTH && serial_history_idx - (i + 1) < 0) return "";
    
    int index = (serial_history_idx - (i + 1)) % (LOG_MAX_LENGTH + 1);
    if (index < 0) index += (LOG_MAX_LENGTH + 1);
    
    return serial_history_timestamps[index];
}


String get_line_i_content_from_serial_history(int i) {
    if (i > LOG_MAX_LENGTH) return "";
    if (serial_history_length < LOG_MAX_LENGTH && serial_history_idx - (i + 1) < 0) return "";
    
    int index = (serial_history_idx - (i + 1)) % (LOG_MAX_LENGTH + 1);
    if (index < 0) index += (LOG_MAX_LENGTH + 1);
    String res = serial_history[index];

    return res;
}