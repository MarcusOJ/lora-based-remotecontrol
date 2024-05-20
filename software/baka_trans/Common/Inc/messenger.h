/*
 * messenger.h
 *
 *  Created on: Apr 7, 2024
 *      Author: mc
 */

#ifndef INC_MESSENGER_H_
#define INC_MESSENGER_H_

#define DIO1_PIN GPIO_PIN_2
#define DIO2_PIN GPIO_PIN_1

extern char get_site_data_cmd[];
extern int rxDone;
extern int txDone;

void get_site_battery_state();
void ping();
void get_site_data();
void radio_message_processor();
void startup_sequence();
void separate_string(const char* input, char* command, char* argument);
void separate_radio_string(const char* input, char* type, char* command, char* argument);
void sync_rtc(char* time);
void radio_transmit();

#endif /* INC_MESSENGER_H_ */
