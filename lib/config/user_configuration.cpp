#include "user_configuration.h"

void print_usb_config(void){
    PRINT_VAR(use_usb);
}

void print_logging_config(void){
    PRINT_VAR(file_duration_minutes);
}

void print_all_user_configs(void){
    SERIAL_USB->println(F("***** all user configs start *****"));

    print_usb_config();
    print_logging_config();

    SERIAL_USB->println(F("***** all user configs end   *****"));
    delay(10);
}