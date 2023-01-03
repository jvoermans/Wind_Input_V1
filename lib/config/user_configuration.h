#ifndef USER_CONFIGURATION_H
#define USER_CONFIGURATION_H

#include <Arduino.h>
#include "firmware_configuration.h"
#include "print_utils.h"

// this is what the user typically wants to change, like frequency of logging, file duration, turning stuff on and off
// this should be possible to modify without changing the hardware setup
// NOTE: should maybe be called "software_configuration" instead

//////////////////////////////////////////////////////////////////////////////////////////
// usb config

// whether to use USB or not
static constexpr bool use_usb {true};

//////////////////////////////////////////////////////////////////////////////////////////
// gnss 

// timeout for getting the first fix
static constexpr uint32_t gnss_fix_timeout_first_fix_milliseconds {1000 * 60 * 5};
// timeout for subsequent fixes
static constexpr uint32_t gnss_fix_timeout_subsequent_fix_milliseconds {1000 * 10};
// number of retries for turning on the GNSS
static constexpr size_t gnss_max_nbr_turn_on_attempts {10};
// number of fixes to average / nsigma for getting a good fix
static constexpr size_t number_gnss_fixes_to_compute_good {10};

//////////////////////////////////////////////////////////////////////////////////////////
// logging

// how often to write to a new file
// TODO: count in number of data blocks instead or something like this
static constexpr uint32_t file_duration_minutes {15};
static constexpr uint32_t file_duration_seconds {60 * 15};
static constexpr uint32_t file_duration_milliseconds {1000 * 60 * 15};

//////////////////////////////////////////////////////////////////////////////////////////
// print all configs

void print_all_user_configs(void);

#endif