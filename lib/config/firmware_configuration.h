#ifndef FIRMWARE_CONFIGURATION_H
#define FIRMWARE_CONFIGURATION_H

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
static constexpr int GNSS_EN_PIN {2};

//////////////////////////////////////////////////////////////////////////////////////////
// pins on the board
// if use specific pins for specific purposes, name them here

static constexpr int LED {13};

//////////////////////////////////////////////////////////////////////////////////////////
// SD card stuff

// pins on the board and frequency for the SD card
static constexpr int SD_CS_PIN {8};
static constexpr int SD_SPI_MHZ {12};

//////////////////////////////////////////////////////////////////////////////////////////
// misc

static constexpr char commit_id[] {STRINGIFY_CONTENT(REPO_COMMIT_ID)};
static constexpr char git_branch[] {STRINGIFY_CONTENT(REPO_GIT_BRANCH)};

//////////////////////////////////////////////////////////////////////////////////////////
// functions

void print_firmware_config(void);

uint64_t read_chip_id(void);

#endif