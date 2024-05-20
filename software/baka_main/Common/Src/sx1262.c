/*
 * sx1262.c
 *
 *  Created on: Feb 8, 2024
 *      Author: mc
 */
#include "sx1262.h"
#include "sx126x.h"
#include "log.h"

sx126x_rx_buffer_status_t rx_buffer_status;
sx126x_pkt_status_lora_t lora_pkt_status;
sx126x_chip_status_t chip_status;
sx126x_sleep_cfgs_t sleep_conf;
sx126x_pkt_type_t packet_type;
sx126x_errors_mask_t errors;
sx126x_standby_cfgs_t standby_conf;
sx126x_cal_mask_t calibrate;
sx126x_stats_lora_t packet_stats;

uint16_t irq_mask;
uint16_t dio1_mask;
uint16_t dio2_mask;
uint16_t dio3_mask;
uint8_t rx_buffer[RX_BUFFER_LEN];

sx126x_cad_params_t cad_params = {
	.cad_detect_min = 10,
	.cad_detect_peak = 22,
	.cad_symb_nb = 0x01,
	.cad_timeout = 3000,
	.cad_exit_mode = 0x01 // CAD_RX - return to RX on successful CAD
};

sx126x_pa_cfg_params_t pa_params = {
	.pa_duty_cycle = 0x04,
	.hp_max = 0x07,
	.device_sel = 0x00,
	.pa_lut = 0x01
};

sx126x_mod_params_lora_t lora_mod_params = {
	.sf = SX126X_LORA_SF7, // Spreading factor
	.cr = SX126X_LORA_CR_4_5, // Coding rate for Forward Error Correction. 4/5 probable best value under normal conditions
	.bw = SX126X_LORA_BW_125, // EU fixed bandwidth 125kHz
	.ldro = 0 // Low data rate optimization off
};

sx126x_pkt_params_lora_t lora_packet_params = {
	.preamble_len_in_symb = 108, // as per recommended in modulation parameters section
	.header_type = SX126X_LORA_PKT_EXPLICIT, // variable length packet
	.pld_len_in_bytes = TEST_STRING_LENGTH, // payload length in bytes
	.crc_is_on = true,
	.invert_iq_is_on = false
};

const void* context;


void sx1262_transmit_constant_wave(){
	sx126x_set_tx_cw(context);
}

void sx1262_standby_rc(){
	standby_conf = SX126X_STANDBY_CFG_RC;
	sx126x_set_standby(context, standby_conf);
}

void sx1262_standby_xosc(){
	standby_conf = SX126X_STANDBY_CFG_XOSC;
	sx126x_set_standby(context, standby_conf);
}

void sx1262_buffer_status(){
	sx126x_get_rx_buffer_status(context, &rx_buffer_status);
	LOG_TRACE("rx buffer start: 0x%02X; length: 0x%02X", rx_buffer_status.buffer_start_pointer, rx_buffer_status.pld_len_in_bytes)
}

uint8_t* sx1262_buffer_read(){
	sx126x_get_rx_buffer_status(context, &rx_buffer_status);
	if(rx_buffer_status.pld_len_in_bytes == 0){ // if read empty buffer then crash
		return rx_buffer;
	}
	sx126x_read_buffer(context, rx_buffer_status.buffer_start_pointer, rx_buffer, rx_buffer_status.pld_len_in_bytes);
	LOG_TRACE("%s", rx_buffer)
	return rx_buffer;
}

void sx1262_packet_stats(){
	sx126x_get_lora_stats(context, &packet_stats);
	LOG_TRACE("CRC Error: %d\nHeader Error: %d\nPacket Received: %d\n", packet_stats.nb_pkt_crc_error, packet_stats.nb_pkt_header_error, packet_stats.nb_pkt_received);
}

void sx1262_lora_packet_status(){
	sx126x_get_lora_pkt_status(context, &lora_pkt_status);
	LOG_TRACE("RSSI: %d\nSignal RSSI: %d\nSNR: %d\n", lora_pkt_status.rssi_pkt_in_dbm, lora_pkt_status.signal_rssi_pkt_in_dbm, lora_pkt_status.snr_pkt_in_db);
}

void get_sx1262_lora_packet_status(sx126x_pkt_status_lora_t *lora_pkt_status){
	sx126x_get_lora_pkt_status(context, lora_pkt_status);
	LOG_TRACE("RSSI: %d\nSignal RSSI: %d\nSNR: %d\n", lora_pkt_status->rssi_pkt_in_dbm, lora_pkt_status->signal_rssi_pkt_in_dbm, lora_pkt_status->snr_pkt_in_db);
}


void sx1262_packet_type(){
	sx1262_get_packet_type();
}

void sx1262_calibrate(){
	calibrate = SX126X_CAL_RC13M;
	sx126x_cal(context, calibrate);
}

void sx1262_transmit(const char* argument){

	sx126x_set_pkt_type(context, SX126X_PKT_TYPE_LORA);

	sx126x_set_rf_freq(context, FREQ); // CEPT/ERC/DEC(01)09 TSMm(2001)32  üldised nõuded

	sx126x_set_pa_cfg(context, &pa_params);

	sx126x_set_tx_params(context, POWER,  SX126X_RAMP_40_US);

	sx126x_set_buffer_base_address(context, TX_BASE_ADDRESS, RX_BASE_ADDRESS);

	sx126x_write_buffer(context, TX_BASE_ADDRESS, (uint8_t*)argument, strlen(argument));

	sx126x_set_lora_mod_params(context, &lora_mod_params);
	lora_packet_params.pld_len_in_bytes = strlen(argument);
	sx126x_set_lora_pkt_params(context, &lora_packet_params);

	irq_mask = SX126X_IRQ_TX_DONE; // enable TxDone interrupt
	dio1_mask = SX126X_IRQ_TX_DONE; // read TxDone on DIO1
	sx126x_set_dio_irq_params(context, irq_mask, dio1_mask, dio2_mask, dio3_mask);

	sx126x_set_tx(context, TX_TIMEOUT);

	LOG_TRACE("TX pld len: %d", lora_packet_params.pld_len_in_bytes);
}

void sx1262_receive(){
	memset(rx_buffer, 0, sizeof(rx_buffer));

	sx126x_set_pkt_type(context, SX126X_PKT_TYPE_LORA);

	sx126x_set_rf_freq(context, FREQ); // CEPT/ERC/DEC(01)09 TSMm(2001)32  üldised nõuded

	sx126x_set_buffer_base_address(context, TX_BASE_ADDRESS, RX_BASE_ADDRESS);

	sx126x_set_lora_mod_params(context, &lora_mod_params);

	sx126x_set_lora_pkt_params(context, &lora_packet_params);

	//sx1262_get_packet_type();

	irq_mask = SX126X_IRQ_RX_DONE;
	dio2_mask = SX126X_IRQ_RX_DONE;
	sx126x_set_dio_irq_params(context, irq_mask, dio1_mask, dio2_mask, dio3_mask);

	sx126x_set_rx(context, 0x00); // continuous mode FFFFFF

	LOG_TRACE("Configured RX")
}

void sx1262_set_irq(){
	irq_mask = SX126X_IRQ_TX_DONE || SX126X_IRQ_RX_DONE; // enable TxDone interrupt
	dio1_mask = SX126X_IRQ_TX_DONE; // read TxDone on DIO1
	dio2_mask = SX126X_IRQ_RX_DONE;
	sx126x_set_dio_irq_params(context, irq_mask, dio1_mask, dio2_mask, dio3_mask);
}

void sx1262_clear_irq(uint16_t irq_mask){
	sx126x_clear_irq_status(context, irq_mask);
}

void sx1262_clear_irq_all(){
	sx126x_clear_irq_status(context, SX126X_IRQ_TX_DONE | SX126X_IRQ_RX_DONE);
}

void sx1262_channel_actitity_detection(){
	sx126x_set_cad_params(context, &cad_params);
	sx126x_set_cad(context);
}

void sx1262_get_device_errors(){

	sx126x_get_device_errors(context, &errors);
	if (errors & SX126X_ERRORS_RC64K_CALIBRATION)
		LOG_TRACE("SX126X_ERRORS_RC64K_CALIBRATION")
	if (errors & SX126X_ERRORS_RC13M_CALIBRATION)
		LOG_TRACE("SX126X_ERRORS_RC13M_CALIBRATION")
	if (errors & SX126X_ERRORS_PLL_CALIBRATION)
		LOG_TRACE("SX126X_ERRORS_PLL_CALIBRATION")
	if (errors & SX126X_ERRORS_ADC_CALIBRATION)
		LOG_TRACE("SX126X_ERRORS_ADC_CALIBRATION")
	if (errors & SX126X_ERRORS_IMG_CALIBRATION)
		LOG_TRACE("SX126X_ERRORS_IMG_CALIBRATION")
	if (errors & SX126X_ERRORS_XOSC_START)
		LOG_TRACE("SX126X_ERRORS_XOSC_START")
	if (errors & SX126X_ERRORS_PLL_LOCK)
		LOG_TRACE("SX126X_ERRORS_PLL_LOCK")
	if (errors & SX126X_ERRORS_PA_RAMP)
		LOG_TRACE("SX126X_ERRORS_PA_RAMP")
	//LOG_ERROR("Undefined error code: 0x%02X", errors)
}

void sx1262_get_status_log(){
	LOG_TRACE("MAINBOARD")
	sx126x_get_status(context, &chip_status);

	switch(chip_status.cmd_status){
		case SX126X_CMD_STATUS_RESERVED:
			LOG_TRACE("SX126X_CMD_STATUS_RESERVED")
			break;
		case SX126X_CMD_STATUS_RFU:
			LOG_TRACE("SX126X_CMD_STATUS_RFU")
			break;
		case SX126X_CMD_STATUS_DATA_AVAILABLE:
			LOG_TRACE("SX126X_CMD_STATUS_DATA_AVAILABLE")
			break;
		case SX126X_CMD_STATUS_CMD_TIMEOUT:
			LOG_TRACE("SX126X_CMD_STATUS_CMD_TIMEOUT")
			break;
		case SX126X_CMD_STATUS_CMD_PROCESS_ERROR:
			LOG_TRACE("SX126X_CMD_STATUS_CMD_PROCESS_ERROR")
			break;
		case SX126X_CMD_STATUS_CMD_EXEC_FAILURE:
			LOG_TRACE("SX126X_CMD_STATUS_CMD_EXEC_FAILURE")
			break;
		case SX126X_CMD_STATUS_CMD_TX_DONE:
			LOG_TRACE("SX126X_CMD_STATUS_CMD_TX_DONE")
			break;
		default:
			LOG_ERROR("Undefined CMD status: %d", chip_status.cmd_status)
	}
	switch(chip_status.chip_mode){
		case SX126X_CHIP_MODE_UNUSED:
			LOG_TRACE("SX126X_CHIP_MODE_UNUSED")
			break;
		case SX126X_CHIP_MODE_RFU:
			LOG_TRACE("SX126X_CHIP_MODE_RFU")
			break;
		case SX126X_CHIP_MODE_STBY_RC:
			LOG_TRACE("SX126X_CHIP_MODE_STBY_RC")
			break;
		case SX126X_CHIP_MODE_STBY_XOSC:
			LOG_TRACE("SX126X_CHIP_MODE_STBY_XOSC")
			break;
		case SX126X_CHIP_MODE_FS:
			LOG_TRACE("SX126X_CHIP_MODE_FS")
			break;
		case SX126X_CHIP_MODE_RX:
			LOG_TRACE("SX126X_CHIP_MODE_RX")
			break;
		case SX126X_CHIP_MODE_TX:
			LOG_TRACE("SX126X_CHIP_MODE_TX")
			break;
		default:
			LOG_ERROR("Undefined CHIP mode: %d", chip_status.chip_mode)
	}

}

void sx1262_get_packet_type(){
	sx126x_get_pkt_type(context, &packet_type);
	switch(packet_type){
		case SX126X_PKT_TYPE_GFSK:
			LOG_TRACE("SX126X_PKT_TYPE_GFSK")
			break;
		case SX126X_PKT_TYPE_LORA:
			LOG_TRACE("SX126X_PKT_TYPE_LORA")
			break;
		case SX126X_PKT_TYPE_LR_FHSS:
			LOG_TRACE("SX126X_PKT_TYPE_LR_FHSS")
			break;
		default:
			LOG_TRACE("Invalid packet type = %d", packet_type)
	}
}

void sx1262_get_irq_status() {
	sx126x_irq_mask_t irq;
	sx126x_get_irq_status(context, &irq);
	if (irq == SX126X_IRQ_NONE)
		LOG_TRACE("SX126X_IRQ_NONE")
	else if (irq == SX126X_IRQ_ALL)
		LOG_TRACE("SX126X_IRQ_ALL")
	else {
		if (irq & SX126X_IRQ_TX_DONE)
			LOG_TRACE("SX126X_IRQ_TX_DONE")
		if (irq & SX126X_IRQ_RX_DONE)
			LOG_TRACE("SX126X_IRQ_RX_DONE")
		if (irq & SX126X_IRQ_PREAMBLE_DETECTED)
			LOG_TRACE("SX126X_IRQ_PREAMBLE_DETECTED")
		if (irq & SX126X_IRQ_SYNC_WORD_VALID)
			LOG_TRACE("SX126X_IRQ_SYNC_WORD_VALID")
		if (irq & SX126X_IRQ_HEADER_VALID)
			LOG_TRACE("SX126X_IRQ_HEADER_VALID")
		if (irq & SX126X_IRQ_HEADER_ERROR)
			LOG_TRACE("SX126X_IRQ_HEADER_ERROR")
		if (irq & SX126X_IRQ_CRC_ERROR)
			LOG_TRACE("SX126X_IRQ_CRC_ERROR")
		if (irq & SX126X_IRQ_CAD_DONE)
			LOG_TRACE("SX126X_IRQ_CAD_DONE")
		if (irq & SX126X_IRQ_CAD_DETECTED)
			LOG_TRACE("SX126X_IRQ_CAD_DETECTED")
		if (irq & SX126X_IRQ_TIMEOUT)
			LOG_TRACE("SX126X_IRQ_TIMEOUT")
		if (irq & SX126X_IRQ_LR_FHSS_HOP)
			LOG_TRACE("SX126X_IRQ_LR_FHSS_HOP")
	}

}
