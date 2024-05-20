/*
 * rtc.c
 *
 *  Created on: May 2, 2024
 *      Author: mc
 */
#include <time.h>
#include "log.h"
#include "rtc_handler.h"

void sync_rtc(char * ret_value, char* time){
	LOG_TRACE("rtc received time: %s", time);

	RTC_TimeTypeDef sTime = {.Seconds = 0,.Minutes = 0, .Hours = 0};
	RTC_DateTypeDef sDate = {.Date = 1, .Month = 1, .Year = 1};
	RTC_TimeTypeDef gTime = {.Seconds = 0,.Minutes = 0, .Hours = 0};
	RTC_DateTypeDef gDate = {.Date = 1, .Month = 1, .Year = 1};

	parseStringToDatetime(time, &sTime, &sDate);
	set_rtc(&sTime, &sDate);
	get_rtc(&gTime, &gDate);

	LOG_TRACE("LOCAL: %d:%d:%d:%d:%d:%d", gTime.Hours, gTime.Minutes, gTime.Seconds, gDate.Date, gDate.Month, gDate.Year);

	snprintf(ret_value, 64, "remotetime-%d:%d:%d:remotedate-%d:%d:%d",gTime.Hours, gTime.Minutes, gTime.Seconds, gDate.Date, gDate.Month, gDate.Year);

}

void set_rtc(RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate){
	LOG_TRACE("Parsed set datetime: %d:%d:%d:%d:%d:%d", sTime->Hours, sTime->Minutes, sTime->Seconds, sDate->Date, sDate->Month, sDate->Year);

	sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime->StoreOperation = RTC_STOREOPERATION_RESET;
	sTime->TimeFormat = RTC_HOURFORMAT_24;
	if (HAL_RTC_SetTime(&hrtc, sTime, RTC_FORMAT_BIN) != HAL_OK) {
		LOG_TRACE("RTC Settime ERROR");
	}
	if (HAL_RTC_SetDate(&hrtc, sDate, RTC_FORMAT_BIN) != HAL_OK) {
		LOG_TRACE("RTC Setdate ERROR");
	}
}

void get_rtc(RTC_TimeTypeDef *gTime, RTC_DateTypeDef *gDate) {
	HAL_RTC_GetTime(&hrtc, gTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, gDate, RTC_FORMAT_BIN);
	LOG_TRACE("GET rtc: %d:%d:%d:%d:%d:%d", gTime->Hours, gTime->Minutes, gTime->Seconds, gDate->Date, gDate->Month, gDate->Year)
}

void parseStringToDatetime(const char* input, RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate){

	struct tm tm;

	if (strptime(input, "%Y-%m-%d %H:%M:%S", &tm) != NULL){ // 2001-11-12 18:31:01

		//LOG_TRACE("Parsed strp datetime: %d:%d:%d:%d:%d:%d", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, tm.tm_mon, tm.tm_year)

		sTime->Hours = tm.tm_hour;
		sTime->Minutes = tm.tm_min;
		sTime->Seconds = tm.tm_sec;

		sDate->Year = tm.tm_year - 100; // years indexed since 1900 xD
		sDate->Month = tm.tm_mon + 1; // indexed from 0
		sDate->Date = tm.tm_mday;

		//LOG_TRACE("Parsed datetime: %d:%d:%d:%d:%d:%d", sTime->Hours, sTime->Minutes, sTime->Seconds, sDate->Date, sDate->Month, sDate->Year)

	} else {
		LOG_TRACE("Could not parse time input: %s", input);
	}
}
