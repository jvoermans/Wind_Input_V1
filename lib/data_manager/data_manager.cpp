#include "data_manager.h"

// GNSS data
GNSS_simple_fix current_fix_start;
GNSS_simple_fix current_fix_end;

// IMU data
// acceleration in the X, Y, Z, North directions
etl::vector<uint16_t, samples_per_channel_per_file> accX;
etl::vector<uint16_t, samples_per_channel_per_file> accY;
etl::vector<uint16_t, samples_per_channel_per_file> accZ;
etl::vector<uint16_t, samples_per_channel_per_file> accD;

// pressure sensor 1 data
etl::vector<uint16_t, samples_per_channel_per_file> press1;

// pressure sensor 2 data
etl::vector<uint16_t, samples_per_channel_per_file> press2;

void DataManager::clear_data(void){
    if (use_usb){
        SERIAL_USB->println(F("clear data manager"));
    }

    accX.clear();
    accY.clear();
    accZ.clear();
    accD.clear();
    press1.clear();
    press2.clear();
}

void DataManager::gather_dataset(void){
    clear_data();

    size_t step = 0;

    while(step < samples_per_channel_per_file){

        step += 1;
        wdt.restart();
    }
}
