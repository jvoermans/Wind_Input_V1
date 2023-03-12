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

#include "sensors_manager.h"

#include "data_manager.h"

#include "blinker.h"

uint8_t status {255};

void setup()
{
    ////////////////////////////////////////////////////////////////////////////////
    // startup delay, to avoid super fast restarts etc; avoid to brick
    delay(10000);

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
    // setup RTC
    board_time_manager.setup_RTC();

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
    // the 3 under are to avoid blinking the SD when the LED blinks
    sleep_for_seconds(1);
    sd_manager_instance.update_filename();
    sd_manager_instance.log_boot();

    ////////////////////////////////////////////////////////////////////////////////
    // start tests to check all is in good order

    // just used for testing if relevant
    if (false){
        enableBurstMode();
        delay(500);
        board_data_manager.gather_dataset();
        disableBurstMode();
        delay(500);
        // sd_manager_instance.update_filename(current_fix_start);
        sd_manager_instance.update_filename();
        sd_manager_instance.log_data();
    }


    if (true){
        // turn the GNSS on and off to "warm up" (and test current use, check if anything wrong, etc)
        gnss_simple_manager_instance.turn_on_off();

        blink(1000, 2);
        delay(2000);
        wdt.restart();

        // turn the sensors on and off to "warm up" (and test that all is well with them)
        board_imu_manger.start_IMU();
        board_imu_manger.stop_IMU();

        blink(1000, 4);
        delay(2000);
        wdt.restart();

        // write a dummy start file just to check the SD card working
        dummy_initialize_fix(current_fix_start);
        // sd_manager_instance.update_filename(current_fix_start);
        sd_manager_instance.update_filename();
        sd_manager_instance.log_boot();

        blink(1000, 6);
        delay(2000);
        wdt.restart();

        // sleep to test that sleep works well and that all starts well after this
        sleep_for_seconds(11);

        blink(1000, 8);
        delay(2000);
        wdt.restart();
    }

    blink(500, 5);
    delay(2000);
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // init the GNSS, get the first GNSS fix;
    // if not GNSS fix, simply go to sleep for 15 minutes before re-trying
    // (note: the GNSS cannot be fully turned out, only put to standby, so not sure
    // how power efficient the sleep actually is, but cannot hurt to sleep)
    // note that this also sets the RTC clock
    if (use_usb){
        SERIAL_USB->println(F("setup init GPS, get first fix, set RTC clock"));
    }

    status = 255;

    gnss_fix_timeout_first_fix_milliseconds = 1000 * 60 * 5;

    while (true){
        status = gnss_simple_manager_instance.get_good_averaged_fix(current_fix_start);
        
        // if we got a fix, exit the loop
        if (status == 0){
            break;
        }

        if (use_usb){
            SERIAL_USB->println(F("Could not find GPS fix; sleep and retry"));
        }

        blink(500, 5);
        delay(2000);
        wdt.restart();

        // else sleep a bit and try again later
        sleep_for_seconds(15*60);
    }

    ////////////////////////////////////////////////////////////////////////////////
    // log a boot message
    if (use_usb){
        SERIAL_USB->println(F("setup write boot message"));
    }
    // sd_manager_instance.update_filename(current_fix_start);
    sd_manager_instance.update_filename();
    sd_manager_instance.log_boot();

    ////////////////////////////////////////////////////////////////////////////////
    // done setup
    if (use_usb){
        SERIAL_USB->println(F("setup done"));
    }

    // reduce to 2 mins the fix max time
    gnss_fix_timeout_first_fix_milliseconds = 1000 * 60 * 3;

    loop_idx = 0;
}

void loop()
{
    ////////////////////////////////////////////////////////////////////////////////
    // done setup
    if (use_usb){
        SERIAL_USB->println(F("loop start"));
    }

    blink(250, 10);
    delay(2000);
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // wait / sleep until next time to start a new file
    board_time_manager.print_status();
    uint32_t current_posix = board_time_manager.get_posix_timestamp();
    uint32_t seconds_to_wait = file_start_modulo_seconds - (current_posix % file_start_modulo_seconds);

    if (use_usb){
        SERIAL_USB->print(F("sleep for (s): ")); SERIAL_USB->println(seconds_to_wait);
    }
    wdt.restart();
    delay(100);

    sleep_for_seconds(seconds_to_wait);

    if (use_usb){
        SERIAL_USB->println(F("start new measurement cycle"));
    }
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // get a GPS fix to get start of file lat, lon, time
    // if (use_usb){
    //     SERIAL_USB->println(F("get start fix"));
    // }
    // wdt.restart();
    // gnss_simple_manager_instance.get_good_averaged_fix(current_fix_start);
    dummy_initialize_fix(current_fix_start);

    ////////////////////////////////////////////////////////////////////////////////
    // we need to take Kalman filter etc; boost
    if (use_usb){
        SERIAL_USB->println(F("enable burst mode"));
    }

    enableBurstMode();
    delay(500);
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // log data for the duration of the file
    if (use_usb){
        SERIAL_USB->println(F("log data"));
    }

    board_data_manager.gather_dataset();

    ////////////////////////////////////////////////////////////////////////////////
    // done need to take Kalman filter etc; deboost
    if (use_usb){
        SERIAL_USB->println(F("disable burst mode"));
    }

    disableBurstMode();
    delay(500);
    wdt.restart();

    ////////////////////////////////////////////////////////////////////////////////
    // get a GPS fix to get end of file lat, lon, time
    // for some reason, the utc clock way of doing here has some issues; set this GPS instead of the start one
    // dummy_initialize_fix(current_fix_end);
    if (use_usb){
        SERIAL_USB->println(F("get end fix"));
    }
    gnss_simple_manager_instance.get_good_averaged_fix(current_fix_end);

    ////////////////////////////////////////////////////////////////////////////////
    // sd card dumping
    if (use_usb){
        SERIAL_USB->println(F("dump data"));
    }
    // TODO: set filename from the UTC clock, not from the GNSS fix (new update_filename() function and use it instead)
    // sd_manager_instance.update_filename(current_fix_start);
    sd_manager_instance.update_filename();
    sd_manager_instance.log_data();

    wdt.restart();
    delay(100);

    if (use_usb){
        SERIAL_USB->println(F("done with logging cycle"));
    }
    delay(100);
    wdt.restart();

    loop_idx += 1;
}