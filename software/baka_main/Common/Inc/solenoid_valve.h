/*
 * solenoid_valve.h
 *
 *  Created on: Feb 9, 2024
 *      Author: mc
 */

#ifndef INC_SOLENOID_VALVE_H_
#define INC_SOLENOID_VALVE_H_

#define FAILSAFE_THRESHOLD 30 // seconds

#include <stdint.h>
#include <gpio.h>

void solenoid_toggle();
void solenoid_failsafe();
uint8_t solenoid_set_state(GPIO_PinState new_state);

extern int solenoid_failsafe_interupt;


#endif /* INC_SOLENOID_VALVE_H_ */
