/*
 * irrigation_controller.h
 *
 *  Created on: May 4, 2024
 *      Author: mc
 */

#ifndef INC_IRRIGATION_CONTROLLER_H_
#define INC_IRRIGATION_CONTROLLER_H_

#include <stdint.h>
#include <rtc.h>

#define PERIOD1_ADDRESS 0x08080000
#define PERIOD2_ADDRESS 0x08080003

typedef struct {
    uint8_t hours;
    uint8_t minutes;
    uint8_t duration_minutes;
    TIM_HandleTypeDef *timer;
} IrrigationTime;

extern int irrigation_stop;
extern IrrigationTime time1;
extern IrrigationTime time2;

void set_irrigation_schedule(char *ret_value, char *input);
uint8_t validate_time(IrrigationTime *time);
uint8_t is_between(uint8_t nr, uint8_t lower, uint8_t upper);
void split_time_string(char *string, IrrigationTime *time1, IrrigationTime *time2);
void start_irrigation();
void stop_irrigation();
uint8_t vaildate_RTC();
void save_irrigation_schedule(IrrigationTime *time1, IrrigationTime *time2);
uint8_t set_irrigation_alarm();
void set_irrigation_duration(TIM_HandleTypeDef *htim, uint8_t seconds);
void set_irrigation_threshold(uint16_t threshold);
void get_irrigation_schedule();
void get_saved_schedule(char *ret_value);
HAL_StatusTypeDef EEPROM_Write(uint32_t address, IrrigationTime *time);
HAL_StatusTypeDef EEPROM_Read(uint32_t address, IrrigationTime *time);

#endif /* INC_IRRIGATION_CONTROLLER_H_ */
