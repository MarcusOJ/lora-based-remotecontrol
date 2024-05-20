/*
 * battery.h
 *
 *  Created on: May 7, 2024
 *      Author: mc
 */

#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_

#include "gpio.h"

#define V_REF 3.3
#define ADC_RESOLUTION 4096
#define R1 3.3
#define R2 4.7

GPIO_PinState get_bat_fault();
GPIO_PinState get_bat_chrg();
void get_battery_state();

#endif /* INC_BATTERY_H_ */
