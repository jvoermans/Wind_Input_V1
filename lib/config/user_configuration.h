#ifndef USER_CONFIGURATION_H
#define USER_CONFIGURATION_H

#include <Arduino.h>
#include "firmware_configuration.h"
#include "print_utils.h"

// for some static asserts
#include <type_traits>

// this is what the user typically wants to change, like frequency of logging, file duration, turning stuff on and off
// this should be possible to modify without changing the hardware setup
// NOTE: should maybe be called "software_configuration" instead

//////////////////////////////////////////////////////////////////////////////////////////
// usb config

// whether to use USB or not
static constexpr bool use_usb {true};

//////////////////////////////////////////////////////////////////////////////////////////
// gnss 

static constexpr bool use_usb_gnss_debug {false};

// timeout for getting the first fix
static constexpr uint32_t gnss_fix_timeout_first_fix_milliseconds {1000 * 60 * 5};
// static constexpr uint32_t gnss_fix_timeout_first_fix_milliseconds {1000 * 5};
// timeout for subsequent fixes
static constexpr uint32_t gnss_fix_timeout_subsequent_fix_milliseconds {1000 * 10};
// number of retries for turning on the GNSS
static constexpr size_t gnss_max_nbr_turn_on_attempts {10};
// number of fixes to average / nsigma for getting a good fix
static constexpr size_t gnss_number_fixes_to_compute_good {10};

static_assert(std::is_same<uint32_t,unsigned long>::value);

//////////////////////////////////////////////////////////////////////////////////////////
// logging

// logging frequency of the data to SD card
static constexpr uint32_t logging_frequency_hz {20};

// how often to write to a new file
// should start when posix_timestamp % file_start_modulo_seconds == 0
// ie to start every 10 minutes: 10 * 60
// TODO: make sure that the correct value is used for deployment under!
static constexpr uint32_t file_start_modulo_seconds {10 * 60};
// static constexpr uint32_t file_start_modulo_seconds {1 * 60};  // for test

// how long duration we actually log
// should be a bit less than the interval between files due to the start modulo, to allow for bookkeeping etc
// ie to log for 8 minutes at a time per file: 8 * 60
// NOTE: reduced to 6 mins, was getting some crashes, wondering if because of low RAM
static constexpr uint32_t file_log_duration_seconds {6 * 60};
// TODO: uncomment under when file_start_modulo_seconds is for true values, comment for tests
static_assert(file_start_modulo_seconds > (file_log_duration_seconds + 60));

// how many samples this corresponds to
static constexpr size_t samples_per_channel_per_file {file_log_duration_seconds * logging_frequency_hz};

// assert this uses less than 75% RAM
// total RAM is 384kB, max use for data storing of 288kB (increase to 290k)
// the (+) is the number of bytes per reading: sum of individual element types in data_manager.h vectors
static_assert((2+2+2+2+2+2+2+2+4+4+4) * samples_per_channel_per_file < 290'000);

static constexpr float data_manager_debug {true};

//////////////////////////////////////////////////////////////////////////////////////////
// sleep

// max sleep duration in seconds
static constexpr uint32_t max_sleep_seconds {60 * 15};
static constexpr bool blink_during_sleep {true};
static constexpr uint32_t seconds_between_sleep_blink {10};
static constexpr uint32_t millis_duration_sleep_blink {100};

//////////////////////////////////////////////////////////////////////////////////////////
// print all configs

void print_all_user_configs(void);

#endif