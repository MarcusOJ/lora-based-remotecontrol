/*
 * log.h
 *
 *  Created on: 9. okt 2021
 *      Author: mjaan
 */

#ifndef INC_LOG_H_
#define INC_LOG_H_

#include "stdint.h"
#include "stdio.h"

#define LOGLEVEL_OFF		(99)
#define LOGLEVEL_TRACE		(1)
#define LOGLEVEL_DEBUG		(2)
#define LOGLEVEL_WARN		(3)
#define LOGLEVEL_ERROR		(4)

#define USELOGLEVEL LOGLEVEL_TRACE

#define BUFFER_SIZE 0xFF

extern char msg_buf[];
//__FILE__, __FUNCTION__, __VA_ARGS__
#define LOG_AT_LOG_LEVEL(LOGLEVEL, TYPE, FORMAT, ...) {\
	if(LOGLEVEL >= USELOGLEVEL){\
		int processor_time = HAL_GetTick();\
		int buf_len = snprintf(msg_buf, BUFFER_SIZE, "%d:%s:%s:%s:"FORMAT"\n", processor_time, TYPE, __FILE__, __FUNCTION__, ##__VA_ARGS__);\
		_log_process_msg(msg_buf, buf_len);\
	}\
}

#define LOG_TRACE(FORMAT, ...)		LOG_AT_LOG_LEVEL(LOGLEVEL_TRACE, "TRACE", FORMAT, ##__VA_ARGS__)
#define LOG_DEBUG(FORMAT, ...)		LOG_AT_LOG_LEVEL(LOGLEVEL_DEBUG, "DEBUG", FORMAT, ##__VA_ARGS__)
#define LOG_WARNING(FORMAT,  ...) 	LOG_AT_LOG_LEVEL(LOGLEVEL_WARN, "WARNING", FORMAT, ##__VA_ARGS__)
#define LOG_ERROR(FORMAT, ...)		LOG_AT_LOG_LEVEL(LOGLEVEL_ERROR, "ERROR", FORMAT, ##__VA_ARGS__)

void _log_process_msg(char *msg, uint16_t len);

#endif /* INC_LOG_H_ */
