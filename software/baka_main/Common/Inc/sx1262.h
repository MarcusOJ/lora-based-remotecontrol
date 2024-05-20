/*
 * sx1262.h
 *
 *  Created on: Feb 8, 2024
 *      Author: mc
 */

#ifndef INC_SX1262_H_
#define INC_SX1262_H_

#include "sx126x.h"

#define TX_BASE_ADDRESS 0x7F
#define RX_BASE_ADDRESS 0x00
#define POWER 22 //dBm -- fixed for sx1262
#define FREQ 8681e5
#define TX_TIMEOUT 1000
#define TEST_STRING_LENGTH 0xFF
#define RX_BUFFER_LEN 0xFF

void sx1262_get_status_log();
void sx1262_set_packet_type();
void sx1262_get_irq_status();
void sx1262_get_device_errors();
void sx1262_transmit_constant_wave();
void sx1262_standby_rc();
void sx1262_standby_xosc();
void sx1262_buffer_status();
uint8_t* sx1262_buffer_read();
void sx1262_packet_stats();
void sx1262_lora_packet_status();
void sx1262_packet_type();
void sx1262_calibrate();
void sx1262_transmit(const char* argument);
void sx1262_receive();
void sx1262_channel_actitity_detection();

#endif /* INC_SX1262_H_ */
