/*
 * messenger.c
 *
 *  Created on: Apr 7, 2024
 *      Author: mc
 */

#include "messenger.h"
#include "gpio.h"
#include "log.h"
#include "sx1262.h"

typedef char* (*MessageHandler)(const char*);

typedef struct {
    char* command;
    MessageHandler handler;
} CommandEntry;

int rxDone = 0;
int txDone = 0;

CommandEntry radioCommandTable[] = {};

void radio_message_processor(){

	char type[255];
	char command[255];
	char argument[255];

	uint8_t *rx_buffer = sx1262_buffer_read();
	//LOG_TRACE("RX Buffer: %s", rx_buffer);

	separate_radio_string(rx_buffer, type, command, argument);

	if(strcmp(type, "cmd") == 0){
		LOG_TRACE("RADIO COMMAND: %s - argument: %s", type, command, argument);
		radio_command_handler(command);
		return;
	}

	if(strcmp(type, "data") == 0){
		LOG_TRACE("RADIO DATA: %s - argument: %s", type, command);
		return;
	}

	LOG_TRACE("NOT RADIO COMMAND: %s - argument: %s", type, command);
	return;
}

void radio_transmit(char *txBuffer){
	sx1262_transmit(txBuffer);
	while(!txDone);
	sx1262_receive();
}

void sync_rtc(char* time){
	char txBuffer[32];

	snprintf(txBuffer, 32, "cmd/set-rtc/%s", time);
	sx1262_transmit(txBuffer);
	while(!txDone);
	sx1262_receive();
}

void get_site_battery_state(){
	sx1262_transmit("cmd/battery-state");
	while(!txDone);
	sx1262_receive();
}

void ping(){
	sx1262_transmit("cmd/ping");
	while(!txDone);
	sx1262_receive();
}

void pong(){
	sx1262_transmit("data/pong");
	while(!txDone);
	sx1262_receive();
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

void get_site_data(){
	sx1262_standby_rc();
	sx1262_transmit("cmd/get-site-data");

	while(!txDone);

	sx1262_receive();
}

void startup_sequence(){
	//sx1262_set_irq();
	sx1262_clear_irq(SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE);
	sx1262_receive();
}

void separate_string(const char* input, char* command, char* argument) {
    // Find the position of the colon in the input string
    const char* colonPos = strchr(input, ':');

    // If colon is found, split the string
    if (colonPos != NULL) {
        // Calculate the length of the command part
        size_t commandLength = colonPos - input;

        // Copy the command part into the 'command' buffer
        strncpy(command, input, commandLength);
        command[commandLength] = '\0'; // Null-terminate the command string

        // Copy the argument part into the 'argument' buffer
        strcpy(argument, colonPos + 1);
    } else {
        // If colon is not found, set both command and argument to the input string
        strcpy(command, input);
        argument[0] = '\0'; // Empty string for argument
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

void radio_command_handler(const char* command, const char* argument, char* result) {
    // Iterate through the command table
    for (size_t i = 0; i < sizeof(radioCommandTable) / sizeof(radioCommandTable[0]); i++) {
        // Check if the command matches any entry in the command table
        if (strcmp(command, radioCommandTable[i].command) == 0) {
            // Call the corresponding handler function
            strcpy(radioCommandTable[i].handler(argument),result);
            return;
        }
    }
    // If no matching command is found
    LOG_TRACE("Unknown radio command: %s\n", command);
}
