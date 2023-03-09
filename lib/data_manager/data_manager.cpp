#include "data_manager.h"

// GNSS data
GNSS_simple_fix current_fix_start;
GNSS_simple_fix current_fix_end;

// IMU data
// acceleration in the X, Y, Z, North directions
etl::vector<uint16_t, samples_per_channel_per_file + 1> accX;
etl::vector<uint16_t, samples_per_channel_per_file + 1> accY;
etl::vector<uint16_t, samples_per_channel_per_file + 1> accZ;
etl::vector<uint32_t, samples_per_channel_per_file + 1> accD;

// pressure sensor 1 and 2 data
etl::vector<uint32_t, samples_per_channel_per_file + 1> press1;
etl::vector<uint32_t, samples_per_channel_per_file + 1> press2;

DataManager board_data_manager;

void DataManager::clear_data(void){
    if (use_usb){
        SERIAL_USB->println(F("clear data manager"));
    }

    // fill with 0s ( so make it easy fo fing metadata from the rest), and clear

    accX.fill(0);
    accX.clear();

    accY.fill(0);
    accY.clear();

    accZ.fill(0);
    accZ.clear();

    accD.fill(0);
    accD.clear();

    press1.fill(0);
    press1.clear();

    press2.fill(0);
    press2.clear();
}

void DataManager::gather_dataset(void){
    clear_data();

    size_t step = 0;

    // r_ holders: readings
    float r_acc_n;
    float r_acc_e;
    float r_acc_d;

    float r_yaw;
    float r_pitch;
    float r_roll;
    
    float r_acc_x;
    float r_acc_y;
    float r_acc_z;

    float r_press_1;
    float r_press_2;

    // e_ holders: entries to the buffers
    uint16_t e_acc_x;
    uint16_t e_acc_y;
    uint16_t e_acc_z;
    uint32_t e_acc_d;

    uint32_t e_press1;
    uint32_t e_press2;

    board_imu_manger.start_IMU();

    unsigned long previous_crrt_millis = millis();
    unsigned long crrt_millis = 0;

    while(step < samples_per_channel_per_file){

        board_imu_manger.get_new_reading(r_acc_n, r_acc_e, r_acc_d, r_yaw, r_pitch, r_roll, r_acc_x, r_acc_y, r_acc_z, r_press_1, r_press_2);

        // normalization accel d
        static constexpr float range_measurement_accel_d = 2.0 * 9.81;  // we have a dynamic range of +-1g centered on g; that should be enough on sea ice
        static constexpr float offset_measurement_accel_d = 0.0;  // d measured is 0g +2g; ie acce_d at rest is +g; need no offset

        // normalization acc x y z
        static constexpr float range_measurement_accel_x_y_z = 4.0 * 9.81;  // x, y, z are -2g +2g as we do not know how the chip is pointing; ie range is 4g and to make positive, add 2g
        static constexpr float offset_measurement_accel_x_y_z = 2.0 * 9.81;  // is -2g +2g; to make positive, add 2g

        // normalization press
        static constexpr float range_press = 250.0;  // sea level pressures; to have some margin, say max is 1100, min 850hPa, ie range 250hPa
        static constexpr float offset_press = -850.0;

        e_acc_x = static_cast<uint16_t>((r_acc_x + offset_measurement_accel_x_y_z) / range_measurement_accel_x_y_z * 65000.0f);
        e_acc_y = static_cast<uint16_t>((r_acc_y + offset_measurement_accel_x_y_z) / range_measurement_accel_x_y_z * 65000.0f);
        e_acc_z = static_cast<uint16_t>((r_acc_z + offset_measurement_accel_x_y_z) / range_measurement_accel_x_y_z * 65000.0f);

        e_acc_d = static_cast<uint32_t>((r_acc_d + offset_measurement_accel_d) / range_measurement_accel_d * 4294900000.0f);

        e_press1 = static_cast<uint32_t>((r_press_1 + offset_press) / range_press * 4294900000.0f);
        e_press2 = static_cast<uint32_t>((r_press_2 + offset_press) / range_press * 4294900000.0f);

        // if want to debug...
        // if (use_usb && data_manager_debug){
        if (false){
            crrt_millis = millis();
            if (crrt_millis-previous_crrt_millis > 1000) {
                PRINTLN_VAR(crrt_millis);

                PRINTLN_VAR(r_acc_x);
                PRINTLN_VAR(e_acc_x);

                PRINTLN_VAR(r_acc_y);
                PRINTLN_VAR(e_acc_y);

                PRINTLN_VAR(r_acc_z);
                PRINTLN_VAR(e_acc_z);

                PRINTLN_VAR(r_acc_d);
                PRINTLN_VAR(e_acc_d);

                PRINTLN_VAR(r_press_1);
                PRINTLN_VAR(e_press1);

                PRINTLN_VAR(r_press_2);
                PRINTLN_VAR(e_press2);

                previous_crrt_millis += 1000;
            }
        }

        accX.push_back(e_acc_x);
        accY.push_back(e_acc_y);
        accZ.push_back(e_acc_z);

        accD.push_back(e_acc_d);

        press1.push_back(e_press1);
        press2.push_back(e_press2);

        step += 1;
        wdt.restart();
    }
}
