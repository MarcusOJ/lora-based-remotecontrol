/*
 * solenoid_valve.c
 *
 *  Created on: Feb 9, 2024
 *      Author: mc
 */

#include "solenoid_valve.h"
#include "gpio.h"
#include "log.h"
#include <tim.h>

int failsafe_counter = 0;
int solenoid_failsafe_interupt = 0;

// add battery check to decline solenoid switch if battery is too low.

void solenoid_toggle(){
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
	GPIO_PinState pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
	LOG_TRACE("Solenoid valve state: %d", pin_state);
}

uint8_t solenoid_set_state(GPIO_PinState new_state){
	GPIO_PinState pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
	if (new_state != pin_state){
		solenoid_toggle();
		return 1;
	}else {
		LOG_TRACE("Solenoid valve state not changed, state: %d", pin_state);
		return 0;
	}
}

void solenoid_failsafe(){
	GPIO_PinState pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
	//LOG_TRACE("Solenoid failsafe - counter: %d", failsafe_counter);
	if (pin_state == 1 && failsafe_counter >= FAILSAFE_THRESHOLD){
		failsafe_counter = 0;
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
		pin_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5);
		LOG_TRACE("Solenoid failsafe reached - new pin state: %d", pin_state);
	}
	if (failsafe_counter > FAILSAFE_THRESHOLD){
		failsafe_counter = 0;
	} else {
		failsafe_counter++;
	}
}

