#include "gnss_manager.h"

Adafruit_GPS adafruit_gps_instance(GNSS_UART);

GNSS_simple_manager gnss_simple_manager_instance;

GNSS_simple_fix current_working_GNSS_simple_fix;

void dummy_initialize_fix(GNSS_simple_fix & to_initialize){
    to_initialize.validity_status = 255;
    to_initialize.year = 9999;
    to_initialize.month = 99;
    to_initialize.day = 99;
    to_initialize.hour = 99;
    to_initialize.minute = 99;
    to_initialize.latitude = 99999;
    to_initialize.longitude = 99999;
    to_initialize.lat_NS = 'X';
    to_initialize.lon_EW = 'X';
    to_initialize.posix_timestamp = 999;
}

void copy_fix(GNSS_simple_fix const & in, GNSS_simple_fix & out){
    out.validity_status = in.validity_status;
    out.year = in.year;
    out.month = in.month;
    out.day = in.day;
    out.hour = in.hour;
    out.minute = in.minute;
    out.latitude = in.latitude;
    out.longitude = in.longitude;
    out.lat_NS = in.lat_NS;
    out.lon_EW = in.lon_EW;
    out.posix_timestamp = in.posix_timestamp;
}

uint8_t GNSS_simple_manager::initialize(void)
{
    dummy_initialize_fix(current_working_GNSS_simple_fix);

    bool success = adafruit_gps_instance.begin(9600);
    if (!success)
    {
        if (use_usb)
        {
            SERIAL_USB->println(F("ERR: cannot .begin GNSS"));
        }
        return 1;
    }

    adafruit_gps_instance.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
    adafruit_gps_instance.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    delay(100);
    wdt.restart();

    adafruit_gps_instance.standby();
    delay(100);
    wdt.restart();

    return 0;
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
        delay(100);
        wdt.restart();
        bool_status = adafruit_gps_instance.wakeup();
        delay(100);
        wdt.restart();
        adafruit_gps_instance.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
        adafruit_gps_instance.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
        delay(100);
        wdt.restart();

        if (!bool_status){
            if (use_usb){
                SERIAL_USB->println(F("ERR: cannot .wakeup GNSS"));
            }

            return 2;
        }

        return 0;
    }
}

uint8_t GNSS_simple_manager::turn_off(void){
    adafruit_gps_instance.standby();
    delay(100);
    wdt.restart();
    GNSS_UART->end();
    delay(100);
    wdt.restart();

    return 0;
}

uint8_t GNSS_simple_manager::get_single_fix(uint32_t timeout_milliseconds, GNSS_simple_fix & output_fix){
    dummy_initialize_fix(output_fix);

    uint32_t start = millis();
    char c;

    // read from the GPS until we time out
    while (millis() - start < timeout_milliseconds){
        wdt.restart();

        // read chars as they arrive
        char c = adafruit_gps_instance.read();

        // at this stage we have received a full NMEA sentence, look at it
        if (adafruit_gps_instance.newNMEAreceived()) {

            // are we able to parse it? if not, continue reading
            if (!adafruit_gps_instance.parse(adafruit_gps_instance.lastNMEA())){
                continue;
            }

            // is this actually a valid fix? if yes, return
            if (adafruit_gps_instance.fix && (adafruit_gps_instance.fixquality > 0)){

                output_fix.validity_status = 0;
                output_fix.year = 2000 + adafruit_gps_instance.year;
                output_fix.month = adafruit_gps_instance.month;
                output_fix.day = adafruit_gps_instance.day;
                output_fix.hour = adafruit_gps_instance.hour;
                output_fix.minute = adafruit_gps_instance.minute;
                output_fix.latitude = adafruit_gps_instance.latitude_fixed;
                output_fix.longitude = adafruit_gps_instance.longitude_fixed;
                output_fix.lat_NS = adafruit_gps_instance.lat;
                output_fix.lon_EW = adafruit_gps_instance.lon;
                kiss_calendar_time calendar_time {output_fix.year, output_fix.month, output_fix.day, output_fix.hour, output_fix.minute, 0};
                output_fix.posix_timestamp = calendar_to_posix(&calendar_time);

                return 0;
            }

        }

    }

    // if we arrived here, we timed out :(
    if (use_usb){
        SERIAL_USB->println(F("ERR: timeout GNSS acquisition"));
    }
    return 3;
}

uint8_t GNSS_simple_manager::get_good_averaged_fix(GNSS_simple_fix & output_fix){
    uint8_t status;

    // turn on
    for (size_t attempt = 0; attempt < gnss_max_nbr_turn_on_attempts; attempt++)
    {
        status = turn_on();

        if (status == 0)
        {
            break;
        }

        delay(100);
        wdt.restart();
    }

    if (status != 0){
        output_fix.validity_status = status;
        return status;
    }

    // get the first fix with the first fix timeout
    status = get_single_fix(gnss_fix_timeout_first_fix_milliseconds, current_working_GNSS_simple_fix);

    if (status != 0){
        return status;
    }

    // get and discard a few more fixes within the update timeout for each of them, to make sure that
    // get time to converge to a good fix
    for (size_t discard = 0; discard < 10; discard++){
        status = get_single_fix(gnss_fix_timeout_subsequent_fix_milliseconds, current_working_GNSS_simple_fix);
        if (status != 0){
            return status;
        }
    }

    // get a few fixes and sigma-sample within the update timeout for each of them
    for (size_t fix_number = 0; fix_number < gnss_number_fixes_to_compute_good; fix_number++){
        status = get_single_fix(gnss_fix_timeout_subsequent_fix_milliseconds, current_working_GNSS_simple_fix);
        if (status != 0){
            return status;
        }

        accumulator_latitude.push_back(current_working_GNSS_simple_fix.latitude);
        accumulator_longitude.push_back(current_working_GNSS_simple_fix.longitude);
        accumulator_posix.push_back(current_working_GNSS_simple_fix.posix_timestamp);
    }

    // we use the latest fit information for everything, except the lat and lon and posix and year month day hour minute: compute n-sigma and update each of these
    copy_fix(current_working_GNSS_simple_fix, output_fix);

    output_fix.latitude = accurate_sigma_filter(accumulator_latitude);
    output_fix.longitude = accurate_sigma_filter(accumulator_longitude);
    output_fix.posix_timestamp = accurate_sigma_filter(accumulator_posix);

    kiss_calendar_time calendar_time_out;
    posix_to_calendar(output_fix.posix_timestamp, &calendar_time_out);
    output_fix.year = calendar_time_out.year;
    output_fix.month = calendar_time_out.month;
    output_fix.day = calendar_time_out.day;
    output_fix.hour = calendar_time_out.hour;
    output_fix.minute = calendar_time_out.minute;

    if (use_usb){
        SERIAL_USB->println(F("--- good average fix start"));
        PRINTLN_VAR(output_fix.posix_timestamp);
        PRINTLN_VAR(output_fix.year);
        PRINTLN_VAR(output_fix.month);
        PRINTLN_VAR(output_fix.day);
        PRINTLN_VAR(output_fix.hour);
        PRINTLN_VAR(output_fix.minute);
        PRINTLN_VAR(output_fix.latitude);
        PRINTLN_VAR(output_fix.lat_NS);
        PRINTLN_VAR(output_fix.longitude);
        PRINTLN_VAR(output_fix.lon_EW);
        SERIAL_USB->println(F("--- good average fix end"));
    }

    wdt.restart();

    // turn off
    status = turn_off();

    return status;
}

uint8_t GNSS_simple_manager::get_good_single_fix_and_set_rtc(GNSS_simple_fix & output_fix){
    uint8_t status;

    // turn on
    for (size_t attempt = 0; attempt < gnss_max_nbr_turn_on_attempts; attempt++)
    {
        status = turn_on();

        if (status == 0)
        {
            break;
        }

        delay(100);
        wdt.restart();
    }

    if (status != 0){
        output_fix.validity_status = status;
        return status;
    }

    // get the first fix with the first fix timeout
    status = get_single_fix(gnss_fix_timeout_first_fix_milliseconds, current_working_GNSS_simple_fix);

    if (status != 0){
        return status;
    }

    // get and discard a few more fixes within the update timeout for each of them, to make sure that
    // get time to converge to a good fix
    for (size_t discard = 0; discard < 10; discard++){
        status = get_single_fix(gnss_fix_timeout_subsequent_fix_milliseconds, current_working_GNSS_simple_fix);
        if (status != 0){
            return status;
        }
    }

    // get the actual fix to use
    status = get_single_fix(gnss_fix_timeout_subsequent_fix_milliseconds, current_working_GNSS_simple_fix);
    if (status != 0){
        return status;
    }

    board_time_manager.set_posix_timestamp(current_working_GNSS_simple_fix.posix_timestamp);

    copy_fix(current_working_GNSS_simple_fix, output_fix);

    if (use_usb){
        SERIAL_USB->println(F("--- good average fix start"));
        PRINTLN_VAR(output_fix.posix_timestamp);
        PRINTLN_VAR(output_fix.year);
        PRINTLN_VAR(output_fix.month);
        PRINTLN_VAR(output_fix.day);
        PRINTLN_VAR(output_fix.hour);
        PRINTLN_VAR(output_fix.minute);
        PRINTLN_VAR(output_fix.latitude);
        PRINTLN_VAR(output_fix.lat_NS);
        PRINTLN_VAR(output_fix.longitude);
        PRINTLN_VAR(output_fix.lon_EW);
        SERIAL_USB->println(F("--- good average fix end"));
    }

    wdt.restart();

    // turn off
    status = turn_off();

    return status;
}
