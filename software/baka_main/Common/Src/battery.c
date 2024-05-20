/*
 * battery.c
 *
 *  Created on: May 7, 2024
 *      Author: mc
 */
#include "battery.h"
#include "gpio.h"
#include <adc.h>
#include "stm32l0xx_hal_adc.h"
#include "log.h"

void get_battery_state(char *ret_value){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET); // first open the divider circuit

	GPIO_PinState chrg_pin = get_bat_chrg();
	GPIO_PinState fault_pin = get_bat_fault();
	uint32_t adc_value = 0;
	float battery_divider_voltage = 0;
	float battery_voltage = 0;

	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;

	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	HAL_ADC_Start(&hadc);
	if (HAL_ADC_PollForConversion(&hadc, 100) == HAL_OK) {
		adc_value = HAL_ADC_GetValue(&hadc);
	}
	HAL_ADC_Stop(&hadc);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);

	battery_divider_voltage = ((float)adc_value / ADC_RESOLUTION) * V_REF;

	battery_voltage = battery_divider_voltage / (R2 / (R1 + R2));

	LOG_TRACE("battery voltage %.2f", battery_voltage);
	snprintf(ret_value, 32, "fault-%d:chrg-%d:voltage-%.2f", fault_pin, chrg_pin, battery_voltage);
}

GPIO_PinState get_bat_fault(){
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
}

GPIO_PinState get_bat_chrg(){
	return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);
}

