/*
 * rfm69.h
 *
 *  Created on: Aug 2, 2021
 *      Author: Vitech-UA
 */

#ifndef INC_RFM69_H_
#define INC_RFM69_H_

#include "main.h"

#define SPI1_DR_8bit          (*(__IO uint8_t *)((uint32_t)&(SPI1->DR)))
#define SPI2_DR_8bit          (*(__IO uint8_t *)((uint32_t)&(SPI2->DR)))

#define RFM69_SELECT_GPIO RFM_NSEL_GPIO_Port
#define RFM69_SELECT_PIN RFM_NSEL_Pin

#define RFM69_RESET_GPIO RFM_RESET_GPIO_Port
#define RFM69_RESET_PIN RFM_RESET_Pin

#define RFM69_SPI_PORT hspi1



void rfm69_select(void);
void rfm69_release(void);
void rfm69_up_reset_pin(void);
void rfm69_down_reset_pin(void);
uint8_t spi_transfer(uint8_t data);

#endif /* INC_RFM69_H_ */
