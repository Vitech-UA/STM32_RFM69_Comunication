/*
 * rfm69.c
 *
 *  Created on: Aug 2, 2021
 *      Author: Vitech-UA
 */

#include "rfm69.h"

extern SPI_HandleTypeDef hspi1;

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

	rfm69_select();
	spi_transfer(reg & 0x7F);
    regval = spi_transfer(0);
	rfm69_release();

	return regval;

}

void rfm69_write_register(uint8_t reg, uint8_t value) {
	rfm69_select();
	spi_transfer(reg | 0x80);
	spi_transfer(value);
	rfm69_release();

}

uint8_t spi_transfer(uint8_t data){
		while (!(RFM69_SPI_PORT.Instance->SR & SPI_SR_TXE))
			; // Очікую спустошення передавального буфера.
		SPI1_DR_8bit = data;

		while (!(RFM69_SPI_PORT.Instance->SR & SPI_SR_RXNE))
			; // Очікую заповнення приймального буфера.
		return (SPI1_DR_8bit);
}
