/*
 * sensor_data.h
 *
 *  Created on: Apr 9, 2024
 *      Author: mc
 */
#ifndef INC_SENSOR_DATA_H_
#define INC_SENSOR_DATA_H_

#include <stdint.h>

typedef struct {
	float airHumidity;
	float airTemperature;
	uint32_t soilMoisture;
} SensorData;

#endif
