/*
 * sht20.c
 *
 *  Created on: Feb 9, 2024
 *      Author: mc
 */
#include "sht20.h"
#include "i2c.h"
#include "log.h"

void display_sht20_temp(){
	LOG_TRACE("temp_f: %.2f", get_sht20_temp());
}

void display_sht20_hum(){
	LOG_TRACE("humidity_f: %.2f", get_sht20_hum());
}

float get_sht20_temp(){
	float temp;
	uint8_t pData[3] = {SHT20_TRIGGER_TEMPERATURE_MEAS, 0, 0};
	if (HAL_I2C_Master_Transmit(&hi2c1, (SHT20_I2C_ADDRESS << 1), pData, 1, HAL_MAX_DELAY) == HAL_OK) {
		// Handle error
		//HAL_I2C_Master_Transmit(&hi2c1, (SHT20_I2C_ADDRESS << 1), pData, 1, HAL_MAX_DELAY);
		HAL_Delay(85);

		//HAL_I2C_Master_Transmit_IT(&hi2c1, dev_address_read, &pData, 8);
		HAL_I2C_Master_Receive(&hi2c1, (SHT20_I2C_ADDRESS << 1) | 0x01, pData, 3, HAL_MAX_DELAY);

		pData[1] = pData[1] & ~SHT20_STATUS_BITS_MASK;	/* zero out status bits */
		uint16_t raw_temp = ((uint16_t)pData[0] << 8) | pData[1];

		temp = -46.85f + 175.72f * (float)raw_temp / 65536.0f;

	} else{
		temp = -1.0;
		LOG_TRACE("SHT20 Transmit Error");
	}
	return temp;
}
float get_sht20_hum(){
	float humidity;
	uint8_t pData[3] = {SHT20_TRIGGER_HUMIDITY_MEAS, 0, 0};
	if (HAL_I2C_Master_Transmit(&hi2c1, (SHT20_I2C_ADDRESS << 1), pData, 1, HAL_MAX_DELAY) == HAL_OK) {

		HAL_Delay(85);

		HAL_I2C_Master_Receive(&hi2c1, (SHT20_I2C_ADDRESS << 1) | 0x01, pData, 3, HAL_MAX_DELAY);

		pData[1] = pData[1] & ~SHT20_STATUS_BITS_MASK;	/* zero out status bits */
		uint16_t raw_humidity = ((uint16_t)pData[0] << 8) | pData[1];

		humidity = -6.0f + 125.0f * (float)raw_humidity / 65536.0f;

	} else{
		humidity = -1.0;
		LOG_TRACE("SHT20 Transmit Error");
	}
	return humidity;
}
