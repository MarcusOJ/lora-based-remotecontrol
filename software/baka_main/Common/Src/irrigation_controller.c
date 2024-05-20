/*
 * irrigation_controller.c
 *
 *  Created on: May 4, 2024
 *      Author: mc
 */

#include "irrigation_controller.h"
#include "solenoid_valve.h"
#include <rtc.h>
#include <tim.h>
#include "log.h"
#include "eeprom_handler.h"

int irrigation_stop = 0;

IrrigationTime time1 = {
		.duration_minutes = 0,
		.hours = 0,
		.minutes = 0,
		.timer = &htim6
};
IrrigationTime time2 = {
		.duration_minutes = 0,
		.hours = 0,
		.minutes = 0,
		.timer = &htim7
};

uint8_t vaildate_RTC(){
	if (1){ // RTC enabled
		return 1;
	}
	return 0;
}

void set_irrigation_schedule(char *ret_value, char *input){
	// communication received function. string format "12:31:15:12:31:15" 2x hours(of day):minutes(of hour):duration_minutes(irrigation duration)
	split_time_string(input, &time1, &time2);

	if (!validate_time(&time1) && !validate_time(&time2)){
		LOG_TRACE("Received invalid conf for irrigation: %d:%d:%d %d:%d:%d", time1.hours, time1.minutes, time1.duration_minutes, time2.hours, time2.minutes, time2.duration_minutes);
		return;
	}
	save_irrigation_schedule(&time1, &time2);
	set_irrigation_alarm();

	snprintf(ret_value, 64, "Irrigation set: %d:%d:%d %d:%d:%d", time1.hours, time1.minutes, time1.duration_minutes, time2.hours, time2.minutes, time2.duration_minutes);
}

uint8_t validate_time(IrrigationTime *time){
	if (is_between(time->hours, 0, 23) && is_between(time->minutes, 0, 59) && is_between(time->duration_minutes, 1, 15)){
		LOG_TRACE("Received conf for irrigation: %d:%d - duration: %d", time->hours, time->minutes, time->duration_minutes);
		return 1;
	}
	return 0;
}

uint8_t is_between(uint8_t nr, uint8_t lower, uint8_t upper){
	if(nr >= lower && nr <= upper){
		return 1;
	}
	return 0;
}

void split_time_string(char *string, IrrigationTime *time1, IrrigationTime *time2){
	char *parts[6];
	const char delimiter[] = ":";
	char *token = strtok(string, delimiter);
	int i = 0;

	// Loop to split the string and store parts
	while (token != NULL && i < 6) {
		parts[i] = token;
		token = strtok(NULL, delimiter);
		i++;
	}

	time1->hours = atoi(parts[0]);
	time1->minutes = atoi(parts[1]);
	time1->duration_minutes = atoi(parts[2]);
	time2->hours = atoi(parts[3]);
	time2->minutes = atoi(parts[4]);
	time2->duration_minutes = atoi(parts[5]);
}

/*
 * start_irrigation
 * Starts the timer that counts until irrigation end
 */
void start_irrigation(IrrigationTime *time){
	if(solenoid_set_state(GPIO_PIN_SET)){
		LOG_TRACE("Irrigation Start for period: %d", time->duration_minutes);
		HAL_TIM_Base_Start_IT(time->timer); // start timer to count end of irrigation. timer stopped in callback
	} else {
		LOG_TRACE("Irrigation In progress");
	}
}

/*
 * stop_irrigation
 * cuts power to the solenoid when timer interrupt is called, flag set in ISR and checked in main loop
 */
void stop_irrigation(){
	if(solenoid_set_state(GPIO_PIN_RESET)){
		LOG_TRACE("Irrigation Stop");
	} else {
		LOG_TRACE("Irrigation Already Stopped");
	}
}

/*
 * set_irrigaiton_schedule
 * saves two irrigation periods in the FLASH to trigger start_irrigation
 */
void save_irrigation_schedule(IrrigationTime *time1, IrrigationTime *time2){
	// calculate seconds from 2 periods and write to eeprom
	// set_irrigation_duration(seconds);
	EEPROM_Write(PERIOD1_ADDRESS, time1);
	EEPROM_Write(PERIOD2_ADDRESS, time2);
	LOG_TRACE("Time written to flash: %d:%d - %d", time1->hours, time1->minutes, time1->duration_minutes);
	LOG_TRACE("Time written to flash: %d:%d - %d", time2->hours, time2->minutes, time2->duration_minutes);

	set_irrigation_duration(time1->timer, time1->duration_minutes * 60); // minutes to seconds
	set_irrigation_duration(time2->timer, time2->duration_minutes * 60);

	set_irrigation_alarm(); // call to set alarm base on set values in eeprom
}

/*
 * get_irrigation_schedule
 * get the irrigation schedule times from eeprom
 */

void get_saved_schedule(char *ret_value){
	IrrigationTime time1 = { .duration_minutes = 0, .hours = 0, .minutes = 0, .timer = &htim6};
	IrrigationTime time2 = { .duration_minutes = 0, .hours = 0, .minutes = 0, .timer = &htim7};

	EEPROM_Read(PERIOD1_ADDRESS, &time1);
	EEPROM_Read(PERIOD2_ADDRESS, &time2);

	snprintf(ret_value, 64, "Saved irrigation schedule:%d:%d-%d,%d:%d-%d", time1.hours, time1.minutes, time1.duration_minutes, time2.hours, time2.minutes, time2.duration_minutes);
}

void get_irrigation_schedule(IrrigationTime *time1, IrrigationTime *time2){
	EEPROM_Read(PERIOD1_ADDRESS, time1);
	EEPROM_Read(PERIOD2_ADDRESS, time2);
	LOG_TRACE("Time in flash: %d:%d - %d", time1->hours, time1->minutes, time1->duration_minutes);
	LOG_TRACE("Time in flash: %d:%d - %d", time2->hours, time2->minutes, time2->duration_minutes);
}

/*
 * irrigation_alarm_set
 * sets alarm in rtc to trigger irrigation start
 */
uint8_t set_irrigation_alarm(){
	// if eeprom contains irrigation schedule then check if alart is set, if not set the set alarms
	// if eeprom does not contain irrigation schedule then log

	IrrigationTime time1 = {0};
	IrrigationTime time2 = {0};

	get_irrigation_schedule(&time1, &time2);

	RTC_AlarmTypeDef sAlarm1 = {0};

	sAlarm1.AlarmTime.Hours = time1.hours;
	sAlarm1.AlarmTime.Minutes = time1.minutes;
	sAlarm1.AlarmTime.Seconds = 0;
	sAlarm1.Alarm = RTC_ALARM_A;
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm1, RTC_FORMAT_BIN);
	LOG_TRACE("Irrigation alarm1: %d:%d", time1.hours, time1.minutes);

	// Set Alarm 2 (for example, 12:02:00 Jan 1, 2024)
	RTC_AlarmTypeDef sAlarm2 = {0};

	sAlarm2.AlarmTime.Hours = time2.hours;
	sAlarm2.AlarmTime.Minutes = time2.minutes;
	sAlarm2.AlarmTime.Seconds = 0;
	sAlarm2.Alarm = RTC_ALARM_B;
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm2, RTC_FORMAT_BIN);
	LOG_TRACE("Irrigation alarm2: %d:%d", time2.hours, time2.minutes);
	return 0;
}

/*
 * set_irrigation_duration
 * sets the duration_minutes of the irrigation in the timer, when the timer reaches reset it triggers an interrupt routine
 */
void set_irrigation_duration(TIM_HandleTypeDef *htim, uint8_t seconds){
	htim->Init.Prescaler = HSI_VALUE - 1; // for 16MHz clock
	htim->Init.Period = seconds;
}

/*
 * set_irrigation_threshold
 * sets the irrigation to follow soil moisture threshold value, if the soil moisture is below the threshold + hysterisis then irrigation will start for 1 minute or until the threshold value is reached
 */
void set_irrigation_threshold(uint16_t threshold){

}

// Function to write data to EEPROM
HAL_StatusTypeDef EEPROM_Write(uint32_t address, IrrigationTime *time) {
	HAL_FLASHEx_DATAEEPROM_Unlock();
	if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address, time->hours) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address + 1, time->minutes) != HAL_OK) {
		return HAL_ERROR;
	}
	if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, address + 2, time->duration_minutes) != HAL_OK) {
		return HAL_ERROR;
	}
	HAL_FLASHEx_DATAEEPROM_Lock();
	return HAL_OK;

}

// Function to read data from EEPROM
HAL_StatusTypeDef EEPROM_Read(uint32_t address, IrrigationTime *time) {
	//HAL_FLASHEx_DATAEEPROM_Unlock(); // Unlock EEPROM
    time->hours = *(__IO uint8_t*)(address);
    time->minutes = *(__IO uint8_t*)(address + 1);
    time->duration_minutes = *(__IO uint8_t*)(address + 2);
    //HAL_FLASHEx_DATAEEPROM_Lock(); // Lock EEPROM
    return HAL_OK;
}

