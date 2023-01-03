// default Arduino libraries
#include <Arduino.h>

// unless specified otherwise, all libraries included are in a lib/ folder of the same name

#include "watchdog_manager.h"

// in the lib/config
#include "firmware_configuration.h"
#include "user_configuration.h"

#include "gnss_manager.h"

uint8_t status {255};

void setup()
{
    ////////////////////////////////////////////////////////////////////////////////
    // startup delay, to avoid super fast restarts etc
    delay(5000);

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
    // TODO: start / check the different sensors to check that all looks good

    ////////////////////////////////////////////////////////////////////////////////
    // init the GNSS, get the first GNSS fix;
    // if not GNSS fix, simply go to sleep for 15 minutes before re-trying
    // (note: the GNSS cannot be fully turned out, only put to standby, so not sure
    // how power efficient the sleep actually is, but cannot hurt to sleep)
    status = 255;

    while (status != 0){
        status = gnss_simple_manager_instance.get_good_simple_fix(current_working_GNSS_simple_fix);
        // TODO: sleep manager
        // TODO: blink LED when well active vs sleep etc - may likely not reach very low power on redboard artemis anyways
    }

}

void loop()
{
    ////////////////////////////////////////////////////////////////////////////////
    // get a GPS fix to get start of file lat, lon, time

    ////////////////////////////////////////////////////////////////////////////////
    // log data for the duration of the file

    ////////////////////////////////////////////////////////////////////////////////
    // get a GPS fix to get end of file lat, lon, time

    ////////////////////////////////////////////////////////////////////////////////
    // sd card dumping

    // generate file name: from datetime of start of the time series
    // YYYY-MM-DD-HH-MM.dat

    // open the new file

    // write to the file

    // close the file
}