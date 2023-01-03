#ifndef FIRMWARE_CONFIGURATION
#define FIRMWARE_CONFIGURATION

#include "Arduino.h"
#include "macro_utils.h"
#include "print_utils.h"

//////////////////////////////////////////////////////////////////////////////////////////
// serial related 

extern Uart * SERIAL_USB;
static constexpr int BAUD_RATE_USB {1000000};

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