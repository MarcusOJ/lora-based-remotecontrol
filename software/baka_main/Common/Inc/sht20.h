/*
 * sht20.h
 *
 *  Created on: Feb 9, 2024
 *      Author: mc
 */

#ifndef INC_SHT20_H_
#define INC_SHT20_H_

#define SHT20_I2C_ADDRESS			0x40
#define SHT20_TRIGGER_TEMPERATURE_MEAS		0xF3
#define SHT20_TRIGGER_HUMIDITY_MEAS		0xF5
#define SHT20_WRITE_USER_REG			0xE6
#define SHT20_READ_USER_REG			0xE7
#define SHT20_STATUS_BITS_MASK			0x03

void display_sht20_temp();
void display_sht20_hum();
float get_sht20_temp();
float get_sht20_hum();

#endif /* INC_SHT20_H_ */
