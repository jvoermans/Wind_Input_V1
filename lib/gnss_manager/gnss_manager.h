#ifndef GNSS_MANAGER
#define GNSS_MANAGER

#include "Arduino.h"

#include "firmware_configuration.h"
#include "user_configuration.h"

#include "watchdog_manager.h"

#include <Adafruit_GPS.h>

// "simple" fix, used for the metadata, filename setting, etc
struct GNSS_simple_fix {
    uint8_t validity_status;  // 0 is valid fix; anything else indicate issues
    uint16_t year;  // natural year: 2022, 2023, etc
    uint8_t month;  // natural month, 1-12, as would be written commonly
    uint8_t day;  // natural day, 1-31, as would be written commonly
    uint8_t hour;  // natural hour, 0-23, as would be written commonly
    uint8_t minute;  // natural minute,  0-59, as would be written commonly
    int32_t latitude;
    int32_t longitude;
};

// dummy initialize a fix with clear outlier values (255 for status, and 9999, 99, or 99999 for data depending on type)
void dummy_initialize_fix(GNSS_simple_fix & to_initialize);

// a GNSS manager to simply:
// get a single, valid, GNSS fix (possibly with some warmup)
// not suitable for accurate continuous measurements over time etc
// automatically switches the GPS on and off
// assumes GNSS is initially off, turns it on, and turns if off again
// ie GNSS is off before and after

// TODO cleanup: should rather clean up the uint8_t status and use an enum class instead
// status table:
// 0 all good
// 1 cannot .begin
// 2 cannot .wakeup
// 255 empty initialized GNSS_simple_fix

class GNSS_simple_manager{
    public:
        // get a simple good fix; note that this may require taking several "raw" fixes to ensure quality
        // this automatically starts the GNSS, takes as many raw fixes as needed to get a good fix, and stops the GNSS (to save power)
        uint8_t get_good_simple_fix(GNSS_simple_fix & output_fix);

    private:  
        // uint8_t used as indications of internal issue; 0 is all is well, other is some issue

        // initialize the gnss: start the uart, the gnss class, set up rates etc, and put in standby mode to save power
        uint8_t initialize(void);

        // turn on and off; in our case, since there is no mosfet to really turn power on off, this is rather standby vs wakeup
        // but if adding a mosfet, that may become a full shutdown
        uint8_t turn_on(void);
        uint8_t turn_off(void);

        // get a single fix, with the GNSS already activated, within a timeout
        // this assumes that the GNSS is already turned on, and does not turn off the GNSS once a fix is gotten
        uint8_t get_single_fix(uint32_t timeout, GNSS_simple_fix & output_fix);

        // is it started, like, has it been initialized including the UART, settings etc?
        bool started {false};

        GNSS_simple_fix current_working_GNSS_simple_fix;

};

extern Adafruit_GPS adafruit_gps_instance;

extern GNSS_simple_manager gnss_simple_manager_instance;

// continuous GNSS measurement would likely only be helpful in order to get an absolute time reference that can be used to correlated in time
// but are not critical to have at the moment
// the simplest would be to make another GNSS manager to do so, if needed in the future

#endif