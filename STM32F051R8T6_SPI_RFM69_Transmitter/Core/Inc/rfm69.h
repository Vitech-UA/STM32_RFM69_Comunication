/*
 * rfm69.h
 *
 *  Created on: Aug 2, 2021
 *      Author: Vitech-UA
 */

#ifndef INC_RFM69_H_
#define INC_RFM69_H_

#include "main.h"
#include "rfm69_registers.h"
#include "stdbool.h"

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
uint8_t rfm69_read_register(uint8_t reg);
uint32_t rfm69_get_frequency(void);
bool rfm69_init(uint8_t freqBand, uint8_t nodeID, uint8_t networkID);
void rfm69_set_mode(uint8_t newMode);
void rfm69_setHighPowerRegs(bool onOff);
void rfm69_encrypt(const char *key);
void rfm69_set_hi_power(bool onOff);
uint8_t rfm69_readTemperature(void);
void rfm69_setPowerLevel(uint8_t powerLevel);
uint8_t rfm69_getPowerLevel();
void rfm69_sleep();
void rfm69_setAddress(uint8_t addr);
// get this node's address
uint8_t rfm69_getAdress();
//set this node's network id
void rfm69_setNetwork(uint8_t networkID);
//get this node's network id
uint8_t rfm69_getNetwork();
void rfm69_sendFrame(uint8_t toAddress, const void *buffer, uint8_t bufferSize,
		bool requestACK, bool sendACK);
int16_t rfm69_readRSSI(bool forceTrigger);
bool rfm69_canSend();
bool rfm69_receiveDone();
#endif /* INC_RFM69_H_ */
