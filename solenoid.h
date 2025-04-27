#ifndef SOLENOID_H
#define SOLENOID_H

#include <avr/io.h>

// Define the solenoid control pin
#define SOLENOID_PORT PORTD
#define SOLENOID_DDR  DDRD
#define SOLENOID_PIN  PD3

// Function declarations
void solenoid_init(void);
void solenoid_on(void);
void solenoid_off(void);

#endif
