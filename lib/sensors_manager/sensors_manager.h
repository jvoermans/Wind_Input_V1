#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include "Arduino.h"

#include <Adafruit_ISM330DHCX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>

#include <Adafruit_AHRS.h>
#include "kiss_clang_3d.h"

#include "Embedded_Template_Library.h"
#include "etl/vector.h"

#include "statistical_processing.h"

#include "watchdog_manager.h"

// NOTE
// this is some ugly hacking / copy paste
// for simplicity of the copy paste, this is still in name an "IMU manager" class, though in reality, it manages
// both IMU and pressure sensor(s)

class IMU_Manager{
    public:
        bool start_IMU();

        // NOTE: we could get more information if we wanted
        bool get_new_reading(float & acc_N_inout, float & acc_E_inout, float & acc_D_inout,
                             float & yaw___inout, float & pitch_inout, float & roll__inout,
                             float & acc_x_inout, float & acc_y_inout, float & acc_z_inout
                            );
        bool stop_IMU();

    private:
        void print_sensors_information(void);
        void set_sensors_parameters(void);
        bool update_accumulate_Kalman(void);

        static constexpr bool imu_use_magnetometer {false};

        // run the Kalman filter at 100Hz
        static constexpr float update_frequency_Kalman_Hz {100.0};
        // read at 833Hz the accel and gyro; i.e. each 1200us
        static constexpr unsigned long nbr_micros_between_accel_gyro_readings {1200UL};
        // read at 560Hz the mag; i.e. each 1785us
        static constexpr unsigned long nbr_micros_between_mag_readings {1785UL};
        // run the Kalman filtering at 100Hz
        static constexpr unsigned long nbr_micros_between_Kalman_update {10000UL};
        // run the output for the wave data at 20Hz (double the usual)
        static constexpr unsigned long nbr_micros_between_IMU_update {50000UL};

        unsigned long time_last_accel_gyro_reading_us {0UL};
        unsigned long time_last_mag_reading_us {0UL};
        unsigned long time_last_Kalman_update_us {0UL};
        unsigned long time_last_IMU_update_us {0UL};

        static constexpr size_t size_dof_accumulators {20};
        //
        etl::vector<float, size_dof_accumulators> accu_acc_x;
        etl::vector<float, size_dof_accumulators> accu_acc_y;
        etl::vector<float, size_dof_accumulators> accu_acc_z;
        //
        etl::vector<float, size_dof_accumulators> accu_gyr_x;
        etl::vector<float, size_dof_accumulators> accu_gyr_y;
        etl::vector<float, size_dof_accumulators> accu_gyr_z;
        //
        etl::vector<float, size_dof_accumulators> accu_mag_x;
        etl::vector<float, size_dof_accumulators> accu_mag_y;
        etl::vector<float, size_dof_accumulators> accu_mag_z;
        //
        // NOTE: ugly, these accs are x y z accs that are at the same speed as the NED (100Hz Kalman output)
        // compare with the other ones that are at the speed of the IMU (833Hz)
        etl::vector<float, size_dof_accumulators> accu_acc_x_new;
        etl::vector<float, size_dof_accumulators> accu_acc_y_new;
        etl::vector<float, size_dof_accumulators> accu_acc_z_new;
        //
        etl::vector<float, size_dof_accumulators> accu_acc_N;
        etl::vector<float, size_dof_accumulators> accu_acc_E;
        etl::vector<float, size_dof_accumulators> accu_acc_D;
        //
        etl::vector<float, size_dof_accumulators> accu_yaw__;
        etl::vector<float, size_dof_accumulators> accu_pitch;
        etl::vector<float, size_dof_accumulators> accu_roll_;

        float acc_x;
        float acc_y;
        float acc_z;

        float gyr_x;
        float gyr_y;
        float gyr_z;

        float mag_x;
        float mag_y;
        float mag_z;

        float acc_N;
        float acc_E;
        float acc_D;

        float yaw__;
        float pitch;
        float roll_;

        float qr;
        float qi;
        float qj;
        float qk;

        Vec3 accel_raw;
        Vec3 accel_NED;
        Quat quat_rotation;

        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;
        sensors_event_t mag; 

        float dummy_inout;

        int stat_nbr_accel_gyro_readings;
        int stat_nbr_mag_readings;
        int stat_nbr_kalman_updates;
};

extern Adafruit_ISM330DHCX ism330dhcx;
extern Adafruit_LIS3MDL lis3mdl;
extern Adafruit_NXPSensorFusion Kalman_filter;
extern IMU_Manager board_imu_manger;

#endif
