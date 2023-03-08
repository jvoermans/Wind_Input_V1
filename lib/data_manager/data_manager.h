#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "gnss_manager.h"

#include "sensors_manager.h"

#include "Embedded_Template_Library.h"
#include "etl/vector.h"

#include "user_configuration.h"

#include "print_utils.h"

// GNSS data
extern GNSS_simple_fix current_fix_start;
extern GNSS_simple_fix current_fix_end;

// ----------

// high frequency buffers
// NOTE: the +1 is not needed, just for helping to find the buffer data end in the binary dumps... hacky but convenient

// IMU data
// acceleration in the X, Y, Z, North directions
extern etl::vector<uint16_t, samples_per_channel_per_file + 1> accX;
extern etl::vector<uint16_t, samples_per_channel_per_file + 1> accY;
extern etl::vector<uint16_t, samples_per_channel_per_file + 1> accZ;
// TODO: enough space to add uint16_t for gyr, yaw, pitch, roll; add here, in the logging, in the dumping, and in the size estimate of user_configuration
extern etl::vector<uint32_t, samples_per_channel_per_file + 1> accD;

// pressure sensor 1 and 2 data
extern etl::vector<uint32_t, samples_per_channel_per_file + 1> press1;
extern etl::vector<uint32_t, samples_per_channel_per_file + 1> press2;

// ----------

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

extern DataManager board_data_manager;

#endif