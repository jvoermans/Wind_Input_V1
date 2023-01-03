#ifndef FIRMWARE_CONFIGURATION
#define FIRMWARE_CONFIGURATION

#include "Arduino.h"
#include "macro_utils.h"
#include "print_utils.h"

// this is what the user typically does not want to change, like what port is used for what, etc
// this should not really be possible to change without changing the hardware (except for a few misc)
// NOTE: should maybe rather be called "hardware_configuration"

//////////////////////////////////////////////////////////////////////////////////////////
// serial related 

extern Uart * SERIAL_USB;
static constexpr int BAUD_RATE_USB {1000000};

//////////////////////////////////////////////////////////////////////////////////////////
// gnss related
extern Uart * GNSS_UART;

//////////////////////////////////////////////////////////////////////////////////////////
// pins on the board
// if use specific pins for specific purposes, name them here

// static constexpr int some_pin_name = PIN_NUMBER; // explanation of pin use

//////////////////////////////////////////////////////////////////////////////////////////
// misc

static constexpr char commit_id[] {STRINGIFY_CONTENT(REPO_COMMIT_ID)};
static constexpr char git_branch[] {STRINGIFY_CONTENT(REPO_GIT_BRANCH)};

//////////////////////////////////////////////////////////////////////////////////////////
// functions

void print_firmware_config(void);

uint64_t read_chip_id(void);

#endif