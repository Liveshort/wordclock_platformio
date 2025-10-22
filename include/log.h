#ifndef _LOG_H_
#define _LOG_H_

#define LOG_MAX_LENGTH 20

class Logger {
    private:
    String _serial_history[LOG_MAX_LENGTH + 1];
    String _serial_history_timestamps[LOG_MAX_LENGTH + 1];
    int _serial_history_idx;
    int _serial_history_length;

    public:
    Logger();
    void print(const String str);
    void println(const String str);
    String get_line_i_timestamp_from_serial_history(int i);
    String get_line_i_content_from_serial_history(int i);
};

#endif