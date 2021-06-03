/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stm32f0xx.h>
#include <stdio.h>
#include "Gpio.h"
#include "SPI.h"
#include "uart.h"
#include "main.h"
#include "RFM69.h"

volatile uint32_t uptime_ms = 0;
Uart Debug = Uart(USART1, 115200);

uint8_t ReceiveRegValue = 0;
uint8_t Temperature = 0;
int32_t freq = 0;
char UART_BUFFER[30];
int bytesReceived = 0;
int main(void) {



	mstimer_init();
	Debug.Printf("Hello\n");
	RFM69 Modem = RFM69(SPI1, GPIOA, 3, true, DataSize_8B);
	Modem.setFrequency(915000000);
	//Modem.readAllRegs();
	Modem.SetResetPin(GPIOA, 4);



	Modem.writeRegister(REG_BITRATEMSB, 0x68);
	Modem.writeRegister(REG_BITRATELSB, 0x2B);

    uint32_t val = Modem.getBitRateKbps();
	Debug.Printf("Current BitRate %i kbps\n", val);

	val = Modem.getFrequency();
	Debug.Printf("Current Frequency %i Hz\n", val);


	Modem.reset();
	Modem.init();
	Modem.sleep();
	Modem.setPowerDBm(13);
	Modem.setCSMA(true);

	char rx[5];

	while (1) {


		 bytesReceived = Modem.receive(rx, sizeof(rx));

		 if (bytesReceived > 0)
		 {
		 Debug.Printf(rx);
		 Debug.SendByte('\n');
		 }

	}
	return 0;
}

extern "C" void SysTick_Handler() {
	uptime_ms++;
}

void delay_ms(unsigned ms) {
	uint32_t start = uptime_ms;
	while (uptime_ms - start < ms)
		;
}

void mstimer_init(void) {
	SysTick_Config(SystemCoreClock / 1000);
}

uint32_t mstimer_get(void) {
	return uptime_ms;
}
