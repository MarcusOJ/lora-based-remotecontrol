/*
 * sen0193.h
 *
 *  Created on: Feb 10, 2024
 *      Author: mc
 */
#include "sen0193.h"
#include "adc.h"
#include "log.h"

void display_soil_moisture(){
	LOG_TRACE("soil: %d", get_soil_moisture());
}

uint32_t get_soil_moisture(){
	// air/dry soil: 3222
	// water/moist soil: 1600'
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
	uint32_t adc_value = 0;
	HAL_ADC_Start(&hadc);
	if (HAL_ADC_PollForConversion(&hadc, 100) == HAL_OK) {
		adc_value = HAL_ADC_GetValue(&hadc);
	}
	HAL_ADC_Stop(&hadc);

	return adc_value;
}
