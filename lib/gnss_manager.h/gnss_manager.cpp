#include "gnss_manager.h"

Adafruit_GPS adafruit_gps_instance(GNSS_UART);

GNSS_simple_manager gnss_simple_manager_instance;

void dummy_initialize_fix(GNSS_simple_fix & to_initialize){
    to_initialize.validity_status = 255;
    to_initialize.year = 9999;
    to_initialize.month = 99;
    to_initialize.day = 99;
    to_initialize.hour = 99;
    to_initialize.minute = 99;
    to_initialize.latitude = 99999;
    to_initialize.longitude = 99999;
}

uint8_t GNSS_simple_manager::initialize(void)
{
    dummy_initialize_fix(current_working_GNSS_simple_fix);

    if (started)
    {
        return 0;
    }

    else
    {
        bool success = adafruit_gps_instance.begin(9600);
        if (!success)
        {
            if (use_usb)
            {
                Serial.println(F("ERR: cannot .begin GNSS"));
            }
            return 1;
        }

        adafruit_gps_instance.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
        adafruit_gps_instance.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

        started = true;

        adafruit_gps_instance.standby();

        wdt.restart();

        return 0;
    }
}

uint8_t GNSS_simple_manager::turn_on(void)
{
    uint8_t status;
    bool bool_status;

    status = initialize();

    if (status != 0)
    {
        return status;
    }

    else
    {
        bool_status = adafruit_gps_instance.wakeup();

        if (!bool_status){
            if (use_usb){
                Serial.println(F("ERR: cannot .wakeup GNSS"));
            }

            return 2;
        }

        return 0;
    }
}

uint8_t GNSS_simple_manager::turn_off(void){
    adafruit_gps_instance.standby();

    return 0;
}

uint8_t GNSS_simple_manager::get_good_simple_fix(GNSS_simple_fix & output_fix){
    uint8_t status;

    // invalid initialize the output
    dummy_initialize_fix(output_fix);

    // turn on
    for (size_t attempt = 0; attempt < gnss_max_nbr_turn_on_attempts; attempt++)
    {
        status = turn_on();

        if (status == 0)
        {
            break;
        }

        delay(100);
    }

    if (status != 0){
        output_fix.validity_status = status;
        return status;
    }

    // get the first fix with the first fix timeout
    status = get_single_fix();

    // get and discard a few more fixes within the update timeout for each of them

    // get a few fixes and sigma-sample within the update timeout for each of them

    // turn off
    status = turn_off();

    return status;
}
