#include "data_manager.h"

// GNSS data
GNSS_simple_fix current_fix_start;
GNSS_simple_fix current_fix_end;

// IMU data
// acceleration in the X, Y, Z, North directions
etl::vector<uint16_t, samples_per_channel_per_file> accX;
etl::vector<uint16_t, samples_per_channel_per_file> accY;
etl::vector<uint16_t, samples_per_channel_per_file> accZ;
etl::vector<uint16_t, samples_per_channel_per_file> accN;

// pressure sensor 1 data
etl::vector<uint16_t, samples_per_channel_per_file> press1;

// pressure sensor 2 data
etl::vector<uint16_t, samples_per_channel_per_file> press2;