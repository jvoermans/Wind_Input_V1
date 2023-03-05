#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "gnss_manager.h"

#include "sensors_manager.h"

#include "Embedded_Template_Library.h"
#include "etl/vector.h"

#include "user_configuration.h"

// GNSS data
extern GNSS_simple_fix current_fix_start;
extern GNSS_simple_fix current_fix_end;

// IMU data
// acceleration in the X, Y, Z, North directions
extern etl::vector<uint16_t, samples_per_channel_per_file> accX;
extern etl::vector<uint16_t, samples_per_channel_per_file> accY;
extern etl::vector<uint16_t, samples_per_channel_per_file> accZ;
extern etl::vector<uint16_t, samples_per_channel_per_file> accD;

// pressure sensor 1 data
extern etl::vector<uint16_t, samples_per_channel_per_file> press1;

// pressure sensor 2 data
extern etl::vector<uint16_t, samples_per_channel_per_file> press2;

class DataManager {
    public:
        // gather a dataset and fill the samples buffers
        void gather_dataset(void);

        // check that all sensors look good and can be started
        // TODO
        // void check_status_sensors(void);

    private:
        // clear all the buffers
        void clear_data(void);
};

#endif