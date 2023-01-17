#ifndef LOGGER_H
#define LOGGER_H

#include "Arduino.h"

#include "data_manager.h"

class Logger {
    public:
        // check all sensors, that they are connected and working;
        // status 0 is success, else is error code
        uint8_t check_all_sensors(void);

        // collect a full dataset, putting it in the data manager buffers
        uint8_t collect_dataset(void);

    private:
        uint8_t start_ism330dhcx(void);
        uint8_t start_pressure_sensors(void);

        uint8_t stop_ism330dhcx(void);
        uint8_t stop_pressure_sensors(void);

        // initialize the ism330dhcx filters
        uint8_t initialize_filters(void);

        // send instruction to start a conversion
        uint8_t pressure_start_measurements(void);
        // collect the conversions in the current_meas_press variables
        uint8_t pressure_get_measurements(void);

        uint16_t current_meas_press1;
        uint16_t current_meas_press2;
};

extern Logger logger_instance;

#endif