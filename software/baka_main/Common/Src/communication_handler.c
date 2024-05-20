/*
 * messenger.c
 *
 *  Created on: Apr 7, 2024
 *      Author: mc
 */

#include "communication_handler.h"
#include "gpio.h"
#include "log.h"
#include "sx1262.h"
#include "rtc_handler.h"
#include "irrigation_controller.h"
#include "solenoid_valve.h"
#include "battery.h"
#include "system.h"

typedef char* (*MessageHandler)(const char*, const char*);

typedef struct {
    char* command;
    MessageHandler handler;
} CommandEntry;

int rxDone = 0;
int txDone = 0;

/*
 * command entry table contains the received command paired with the called function
 * the function will accept the return value (char *) and argument (char *) as first and second argument respectively
 */

CommandEntry radioCommandTable[] = {
	{"get-site-data", collect_site_data},
	{"get-battery-state", get_battery_state},
	{"get-irrigation-schedule", get_saved_schedule},
	{"set-solenoid", solenoid_toggle},
	{"set-rtc", sync_rtc},
	{"set-irrigation-schedule", set_irrigation_schedule},
	{"ping", pong}
};

void radio_transmit(char *message){
	char txBuffer[64];
	snprintf(txBuffer, 64, "data/device_id-%d:%s", device_id, message);
	LOG_TRACE("transmitting: %s", txBuffer);
	sx1262_transmit(txBuffer);
	while(!txDone);
	sx1262_receive();
	memset(txBuffer, '\0', sizeof(txBuffer));
}

void radio_message_processor(){

	char type[255];
	char command[255];
	char argument[255];

	uint8_t *rx_buffer = sx1262_buffer_read();
	//LOG_TRACE("RX Buffer: %s", rx_buffer);

	//separateString(rx_buffer, type, command);
	separate_radio_string(rx_buffer, type, command, argument);

	if(strcmp(type, "cmd") == 0){
		LOG_TRACE("RADIO COMMAND: %s - argument: %s", command, argument);
		radio_command_handler(command, argument);
		return;
	}

	if(strcmp(type, "data") == 0){
		LOG_TRACE("RADIO DATA: %s - argument: %s", type, command);
		return;
	}

	LOG_TRACE("NOT RADIO COMMAND: %s - argument: %s", type, command);
	sx1262_receive();
	return;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	switch (GPIO_Pin){
		case DIO1_PIN:
			txDone = 1;
			break;
		case DIO2_PIN:
			rxDone = 1;
			break;
	}
}

void separate_string(const char* input, char* type, char* command) {
    const char* colonPos = strchr(input, ':');
    if (colonPos != NULL) {
        size_t commandLength = colonPos - input;
        strncpy(type, input, commandLength);
        type[commandLength] = '\0';
        strcpy(command, colonPos + 1);
    }
    else {
        strcpy(type, input);
        command[0] = '\0';
    }
}

void separate_radio_string(const char* input, char* type, char* command, char* argument){
	const char *delimiter = "/";
	char *token;

	token = strtok(input, delimiter);
	if (token != NULL) {
		strcpy(type, token);
		token = strtok(NULL, delimiter);
		if (token != NULL) {
			strcpy(command, token);
			token = strtok(NULL, delimiter);
			if (token != NULL) {
				strcpy(argument, token);
			}
		}
	}

}

void radio_command_handler(const char* command, const char* argument){
	char * result[64];
    for (size_t i = 0; i < sizeof(radioCommandTable) / sizeof(radioCommandTable[0]); i++) {
        if (strcmp(command, radioCommandTable[i].command) == 0) {
            radioCommandTable[i].handler(result, argument);
            radio_transmit(result);
            return;
        }
    }
    LOG_TRACE("Unknown radio command: %s\n", command);
    sx1262_receive();
}
