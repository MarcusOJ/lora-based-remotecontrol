/*
 * sx1262_hal.c
 *
 *  Created on: Dec 31, 2023
 *      Author: mc
 */
#include "sx126x_hal.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"

uint8_t trans_busy(){
	//HAL_Delay(100);
	GPIO_PinState busy_pin = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
	//LOG_TRACE("busy: %d", busy_pin);
	if (busy_pin == GPIO_PIN_SET){
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
		return 1;
	}
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	return 0;
}

sx126x_hal_status_t sx126x_hal_write( const void* context, const uint8_t* command, const uint16_t command_length, const uint8_t* data, const uint16_t data_length ){

	while(trans_busy());

	uint16_t total_length = command_length + data_length;

	// Create a buffer to hold the combined command and data
	uint8_t tx_buffer[total_length];

	// Copy the command to the buffer
	memcpy(tx_buffer, command, command_length);

	// If there's data, copy it to the buffer after the command
	if (data != NULL && data_length > 0) {
		memcpy(&tx_buffer[command_length], data, data_length);
	}
	// Transmit the combined buffer
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // Set NSS low
	HAL_SPI_Transmit(&hspi2, tx_buffer, total_length, 1000); // Transmit command and data
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // Set NSS high


	return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read( const void* context, const uint8_t* command, const uint16_t command_length, uint8_t* data, const uint16_t data_length ){

	while(trans_busy());
	// something something timeout
	// return SX126X_HAL_STATUS_ERROR;

	//SPI_TransmitReceive(command, command_length, data, data_length);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 0);
	HAL_SPI_Transmit(&hspi2, (uint8_t*)command, command_length, 1000);
	HAL_SPI_Receive(&hspi2, (uint8_t*) data, data_length, 1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, 1);

	return SX126X_HAL_STATUS_OK;
}


sx126x_hal_status_t sx126x_hal_reset( const void* context );

sx126x_hal_status_t sx126x_hal_wakeup( const void* context );
