#ifndef BLINKER_H
#define BLINKER_H

#include "Arduino.h"
#include "firmware_configuration.h"
#include "watchdog_manager.h"

void blink(unsigned long millis_blink_period, size_t nbr_blinks);

#endif