/*
 * rtc.h
 *
 *  Created on: May 3, 2024
 *      Author: mc
 */

#ifndef INC_RTC_HANDLER_H_
#define INC_RTC_HANDLER_H_

#include <rtc.h>

void set_rtc(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
void get_rtc(RTC_TimeTypeDef *gTime, RTC_DateTypeDef *gDate);
void parseStringToDatetime(const char* input, RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
void sync_rtc(char * ret_value, char* time);

#endif /* INC_RTC_HANDLER_H_ */
