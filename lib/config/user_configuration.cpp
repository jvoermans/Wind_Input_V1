#include "user_configuration.h"

void print_usb_config(void){
    PRINTLN_VAR(use_usb);
}

void print_gnss_config(void){
    PRINTLN_VAR(gnss_fix_timeout_first_fix_milliseconds);
    PRINTLN_VAR(gnss_fix_timeout_subsequent_fix_milliseconds);
    PRINTLN_VAR(gnss_max_nbr_turn_on_attempts);
    PRINTLN_VAR(gnss_number_fixes_to_compute_good);
}

void print_logging_config(void){
    PRINTLN_VAR(file_start_modulo_seconds);
}

void print_all_user_configs(void){
    SERIAL_USB->println(F("***** all user configs start *****"));

    print_usb_config();
    print_logging_config();

    SERIAL_USB->println(F("***** all user configs end   *****"));
    delay(10);
}