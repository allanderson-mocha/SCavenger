#include "solenoid.h"

void solenoid_init(void) {
    // Set SOLENOID_PIN as output
    SOLENOID_DDR |= (1 << SOLENOID_PIN);
    // Make sure solenoid starts OFF
    solenoid_off();
}

void solenoid_on(void) {
    // Set the solenoid pin HIGH to turn it ON
    SOLENOID_PORT |= (1 << SOLENOID_PIN);
}

void solenoid_off(void) {
    // Set the solenoid pin LOW to turn it OFF
    SOLENOID_PORT &= ~(1 << SOLENOID_PIN);
}
