#include "blinker.h"

void blink(unsigned long millis_blink_period, size_t nbr_blinks){
    pinMode(LED, OUTPUT);

    for (int i=0; i<nbr_blinks; i++){
        digitalWrite(LED, HIGH);
        delay(millis_blink_period / 2);
        wdt.restart();
        digitalWrite(LED, LOW);
        delay(millis_blink_period / 2);
        wdt.restart();
    }

    pinMode(LED, INPUT);
}