#ifndef _TIME_FUNCTIONS_H_
#define _TIME_FUNCTIONS_H_

void initialize_sntp_time_servers();
bool check_time_synchronization();
bool update_time();
void set_current_time_to_target_time();

#endif