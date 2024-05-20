/*
 * sensor_data.c
 *
 *  Created on: Apr 9, 2024
 *      Author: mc
 */
#include "sht20.h"
#include "sen0193.h"
#include "sensor_data.h"
#include "log.h"


void update_sensor_data(SensorData *sensorData){
	LOG_TRACE("ah-%.2f:at-%.2f:sm-%d", get_sht20_hum(), get_sht20_temp(), get_soil_moisture());
	sensorData->soilMoisture = get_soil_moisture();
	sensorData->airTemperature = get_sht20_temp();
	sensorData->airHumidity = get_sht20_hum();
}
