#ifndef GNSS_MANAGER
#define GNSS_MANAGER

#include "Arduino.h"

#include "firmware_configuration.h"
#include "user_configuration.h"

#include "watchdog_manager.h"

#include <Adafruit_GPS.h>

#include "Embedded_Template_Library.h"
#include "etl/vector.h"

#include "statistical_processing.h"

#include "kiss_posix_time_utils.hpp"

#include "time_manager.h"

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
    // note: not sure if the latitude and longitude from adafruit library have sign or abs, add NS and EW chars...
    char lat_NS;
    char lon_EW;
    uint32_t posix_timestamp;
};

// dummy initialize a fix with clear outlier values (255 for status, and 9999, 99, or 99999 for data depending on type)
void dummy_initialize_fix(GNSS_simple_fix & to_initialize);

void copy_fix(GNSS_simple_fix const & in, GNSS_simple_fix & out);

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
// 3 cannot get a valid fix before timeout
// 255 empty initialized GNSS_simple_fix

class GNSS_simple_manager{
    public:
        // turn on and off, to make sure it is off from the start
        void turn_on_off(void);

        // get a good averaged fix; note that this may require taking several "raw" fixes to ensure quality
        // this automatically starts the GNSS, takes as many raw fixes as needed to get a good average fix, and stops the GNSS (to save power)
        // this also sets the RTC clock using the last fix
        uint8_t get_good_averaged_fix(GNSS_simple_fix & output_fix);

    private:  
        // uint8_t used as indications of internal issue; 0 is all is well, other is some issue

        // turn on and off
        uint8_t turn_on(void);
        uint8_t turn_off(void);

        // get a single fix, with the GNSS already activated, within a timeout
        // this assumes that the GNSS is already turned on, and does not turn off the GNSS once a fix is gotten
        uint8_t get_single_fix(uint32_t timeout_milliseconds, GNSS_simple_fix & output_fix);

        etl::vector<int32_t, gnss_number_fixes_to_compute_good> accumulator_latitude;
        etl::vector<int32_t, gnss_number_fixes_to_compute_good> accumulator_longitude;
        etl::vector<int32_t, gnss_number_fixes_to_compute_good> accumulator_posix;
};

extern Adafruit_GPS adafruit_gps_instance;

extern GNSS_simple_manager gnss_simple_manager_instance;

extern GNSS_simple_fix current_working_GNSS_simple_fix;

// continuous GNSS measurement would likely only be helpful in order to get an absolute time reference that can be used to correlated in time
// but are not critical to have at the moment
// the simplest would be to make another GNSS manager to do so, if needed in the future

#endif