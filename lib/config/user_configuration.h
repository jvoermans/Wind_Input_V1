#ifndef USER_CONFIGURATION_H
#define USER_CONFIGURATION_H

#include <Arduino.h>
#include "firmware_configuration.h"
#include "print_utils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// usb config

// whether to use USB or not
static constexpr bool use_usb {true};

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