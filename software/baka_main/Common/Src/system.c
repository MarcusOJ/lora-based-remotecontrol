/*
 * system.c
 *
 *  Created on: May 7, 2024
 *      Author: mc
 */
#include "system.h"
#include "sx1262.h"
#include "sensor_data.h"


void pong(char *ret_value){
	sx126x_pkt_status_lora_t lora_pkt_status = {
			.rssi_pkt_in_dbm = 0,
			.signal_rssi_pkt_in_dbm = 0,
			.snr_pkt_in_db = 0
	};
	get_sx1262_lora_packet_status(&lora_pkt_status);
	snprintf(ret_value, 64, "pong-rssi:%d-srssi:%d-snr:%d", lora_pkt_status.rssi_pkt_in_dbm, lora_pkt_status.signal_rssi_pkt_in_dbm, lora_pkt_status.snr_pkt_in_db);
}

void collect_site_data(char *ret_value){
	SensorData sensorData = {
			.airHumidity = 0,
			.airTemperature = 0,
			.soilMoisture = 0
	};
	update_sensor_data(&sensorData);

	snprintf(ret_value, 64, "ah-%.2f:at-%.2f:sm-%d", sensorData.airHumidity, sensorData.airTemperature, sensorData.soilMoisture);

}

void startup_sequence(){
	//sx1262_set_irq();
	set_irrigation_alarm();
	sx1262_clear_irq(SX126X_IRQ_RX_DONE | SX126X_IRQ_TX_DONE);
	sx1262_receive();
}
