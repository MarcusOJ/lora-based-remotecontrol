/*
 * log.c
 *
 *  Created on: 7. okt 2022
 *      Author: mjaan
 */
#include "log.h"
#include "usbd_cdc_if.h"

char msg_buf[BUFFER_SIZE];

void _log_process_msg(char *msg, uint16_t len){
	 CDC_Transmit_FS((uint8_t*) msg, len);
}
