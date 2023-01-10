// default Arduino libraries
#include <Arduino.h>

// unless specified otherwise, all libraries included are in a lib/ folder of the same name

#include "watchdog_manager.h"

// in the lib/config
#include "firmware_configuration.h"
#include "user_configuration.h"

#include "gnss_manager.h"

#include "time_manager.h"
#include "sleep_manager.h"

#include "sd_configuration.h"
#include "sd_manager.h"

#include "data_manager.h"

uint8_t status {255};

void setup()
{
    ////////////////////////////////////////////////////////////////////////////////
    // startup delay, to avoid super fast restarts etc
    delay(1000);

    ////////////////////////////////////////////////////////////////////////////////
    // setup watchdog
    // for wdt setup see: https://github.com/sparkfun/Arduino_Apollo3/blob/main/libraries/WDT/examples/Example2_WDT_Config/Example2_WDT_Config.ino
    // wdt generate tics at 1 second
    // generate a wdt interrupt every 32 tics (we do not use it anyways)
    // generate a wdt reset every 32 tics (this will reboot after 32 seconds)
    wdt.configure(WDT_1HZ, 32, 32);
    wdt.start();
    // from now on, need to wdt.restart(); at least every 32 seconds, else reboot

    ////////////////////////////////////////////////////////////////////////////////
    // start USB and standard prints, if relevant
    if (use_usb){
        SERIAL_USB->begin(BAUD_RATE_USB);
        delay(100);
        wdt.restart();

        print_firmware_config();
        wdt.restart();
        print_all_user_configs();
        wdt.restart();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // TODO: CRITICAL: start / check the different sensors and the SD card to check that all looks good

    ////////////////////////////////////////////////////////////////////////////////
    // init the GNSS, get the first GNSS fix;
    // if not GNSS fix, simply go to sleep for 15 minutes before re-trying
    // (note: the GNSS cannot be fully turned out, only put to standby, so not sure
    // how power efficient the sleep actually is, but cannot hurt to sleep)
    status = 255;

    while (true){
        status = gnss_simple_manager_instance.get_good_averaged_fix(current_working_GNSS_simple_fix);
        
        // if we got a fix, exit the loop
        if (status == 0){
            break;
        }

        // else sleep a bit and try again later
        sleep_for_seconds(15*60);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // set the time manager
    // TODO: BONUS: if too many failures, either reboot or just go further
    while(gnss_simple_manager_instance.get_good_single_fix_and_set_rtc(current_working_GNSS_simple_fix) != 0){delay(1000);};

    ////////////////////////////////////////////////////////////////////////////////
    // log a boot message
    sd_manager_instance.update_filename(current_working_GNSS_simple_fix);
    sd_manager_instance.log_boot();
}

void loop()
{
    ////////////////////////////////////////////////////////////////////////////////
    // wait / sleep until next time to start a new file
    board_time_manager.print_status();
    uint32_t current_posix = board_time_manager.get_posix_timestamp();
    uint32_t seconds_to_wait = file_start_modulo_seconds - (current_posix % file_start_modulo_seconds);
    sleep_for_seconds(seconds_to_wait);

    if (use_usb){
        SERIAL_USB->println(F("start new measurement cycle"));
    }
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // get a GPS fix to get start of file lat, lon, time
    // TODO: BONUS: if too many failures, either reboot or just go further
    while (gnss_simple_manager_instance.get_good_averaged_fix(current_fix_start) != 0){
        delay(1000);
        wdt.restart();
    }

    ////////////////////////////////////////////////////////////////////////////////
    // we need to take Kalman filter etc; boost
    enableBurstMode();

    ////////////////////////////////////////////////////////////////////////////////
    // log data for the duration of the file

    delay(5000);  // dummy delay just for now
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // done need to take Kalman filter etc; deboost
    disableBurstMode();

    ////////////////////////////////////////////////////////////////////////////////
    // get a GPS fix to get end of file lat, lon, time
    // TODO: BONUS: if too many failures, either reboot or just go further
    while (gnss_simple_manager_instance.get_good_averaged_fix(current_fix_end) != 0){
        delay(1000);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // sd card dumping
    sd_manager_instance.update_filename(current_fix_start);
    sd_manager_instance.log_data();

}