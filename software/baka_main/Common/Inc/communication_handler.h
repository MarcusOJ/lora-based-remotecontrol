/*
 * messenger.h
 *
 *  Created on: Apr 7, 2024
 *      Author: mc
 */

#ifndef INC_COMMUNICATION_HANDLER_H_
#define INC_COMMUNICATION_HANDLER_H_

#define DIO1_PIN GPIO_PIN_10
#define DIO2_PIN GPIO_PIN_9
#define DIO3_PIN GPIO_PIN_8
#define DEVICE_ID 1

extern int rxDone;
extern int txDone;

void radio_message_processor();
void radio_transmit(char *message);
void separate_string(const char* input, char* type, char* command);
void separate_radio_string(const char* input, char* type, char* command, char* argument);
void radio_command_handler(const char* command, const char* argument);

#endif /* INC_COMMUNICATION_HANDLER_H_ */
