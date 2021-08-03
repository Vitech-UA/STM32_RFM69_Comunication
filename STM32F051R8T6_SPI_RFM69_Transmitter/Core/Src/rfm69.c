/*
 * rfm69.c
 *
 *  Created on: Aug 2, 2021
 *      Author: Vitech-UA
 */

#include "rfm69.h"

extern SPI_HandleTypeDef hspi1;
// Clock constants. DO NOT CHANGE THESE!
#define RFM69_XO               32000000    ///< Internal clock frequency [Hz]
#define RFM69_FSTEP            61.03515625 ///< Step width of synthesizer [Hz]

void rfm69_select(void) {

	RFM69_SELECT_GPIO->BSRR |= RFM69_SELECT_PIN << 16U; // RESET

}

void rfm69_release(void) {

	RFM69_SELECT_GPIO->BSRR |= RFM69_SELECT_PIN; // SET
}

void rfm69_up_reset_pin(void) {
	RFM69_RESET_GPIO->BSRR |= RFM69_RESET_PIN; //SET
}
void rfm69_down_reset_pin(void) {
	RFM69_RESET_GPIO->BSRR |= RFM69_RESET_PIN << 16U; //SET
}

uint8_t rfm69_read_register(uint8_t reg) {

	uint8_t regval = 0;
	uint8_t zero_byte = 0;
	uint8_t read_data = reg & 0x7F;

	rfm69_select();
	HAL_SPI_Transmit(&hspi1, &read_data, 1, 100);
	HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&zero_byte , (uint8_t*)&regval, 1, 100);
	rfm69_release();

	return regval;

}

void rfm69_write_register(uint8_t reg, uint8_t value) {
	rfm69_select();
	uint8_t write_data = reg | 0x80;
	HAL_SPI_Transmit(&hspi1, (uint8_t*) &write_data, 1, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t*) &value, 1, 100);
	rfm69_release();

}

uint32_t rfm69_get_frequency(void){
	return RFM69_FSTEP
				* (((uint32_t)rfm69_read_register(REG_FRFMSB) << 16)
						+ ((uint16_t) rfm69_read_register(REG_FRFMID) << 8)
						+ rfm69_read_register(REG_FRFLSB));
}

void rfm69_set_frequency(unsigned int frequency) {

	// calculate register value
	frequency /= RFM69_FSTEP;

	// set new frequency
	rfm69_write_register(0x07, frequency >> 16);
	rfm69_write_register(0x08, frequency >> 8);
	rfm69_write_register(0x09, frequency);
}
