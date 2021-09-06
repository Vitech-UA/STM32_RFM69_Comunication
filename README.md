# STM32 RFM69 HAL Library
Проєкт бібліотеки радіомодуля RFM69 на С, для мікроконтролерів STM32. Використовує HAL.
Протестовано на STM32F051R8 та STM32F334R8.
Модулі для відладки купував тут: 
 - RFM: https://www.rcscomponents.kiev.ua/product/rfm69w-433s2-hoperf-trx-module_54636.html
 - Плата-адаптер: https://www.rcscomponents.kiev.ua/product/hopeduino-adapter-dlya-radiomodulya-rfm69hw_126909.html
 - Антена 433 Мгц: https://www.rcscomponents.kiev.ua/product/sma-antenna-433mhz-40901000005_121801.html

# Установка
1) Тягнемо у проєкт файли: 
rfm69_registers.h
rfm69.h
rfm69.с
2) інклудимо rfm69.h в main.c
3) Описуємо необхідні налаштування.
 ``` c
        #define MYID        0xCB    /* (range up to 254)*/
        #define NETWORKID     0x10  /* (range up to 255)*/
        #define GATEWAYID     0xAB
        #define FREQUENCY   RF69_433MHZ
        #define ENCRYPTKEY    "sampleEncryptKey"
```
4)
    a) Для передавача оголошуємо буфер, Ініціалізуємо модуль. Сетимо за необхідності ENCRYPTKEY, І пуляємо дані в ефір
```C
    char data_to_transmit[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x01, 0x02,
			0x03 };
	rfm69_down_reset_pin();
	if (rfm69_init(FREQUENCY, MYID, NETWORKID)) 
	{
		setFrequency(433000000);
	}
	if (!setAESEncryption(ENCRYPTKEY, 16)) {
		Error_Handler();
	}
	send(GATEWAYID, (uint8_t*) &data_to_transmit, sizeof(data_to_transmit),false, true);
```
b) Для приймача, оголошуємо елемент структури Payload і ресівимо дані в його поля.
```c
    Payload receive_data;
	receiveBegin();

	while (1) {

		if (waitForResponce(&receive_data, 1000)) {
			sprintf(RxBuffer,
					"-------------------------------\r\n"
					"Received from sender: 0x%X\r\n"
					"Received length: %d bytes\r\n"
					"RSSI: %d\r\n"
					"Receive to: 0x%X\r\n"
					"CTL_BYTE: 0x%X\r\n",
					receive_data.senderId, receive_data.size,
					receive_data.signalStrength,receive_data.targetId, receive_data.ctlByte);

			HAL_UART_Transmit(&huart1, RxBuffer, strlen(RxBuffer), 100);
			for (uint8_t i = 0; i <= receive_data.size; i++) {
				sprintf(RxBuffer, "Data[%d]: 0x%X\r\n", i, receive_data.data[i]);
				HAL_UART_Transmit(&huart1, RxBuffer, strlen(RxBuffer), 100);
			}
```

5) Призначення виводів STM32
![Снимок](https://user-images.githubusercontent.com/74230330/128074874-38a1fd05-f855-40c2-ac01-00b0569efd6f.JPG)
6) Налаштування SPI
![SPI_Settings](https://user-images.githubusercontent.com/74230330/128081021-bec453f8-cec2-41f5-8124-864e07bdbdc8.JPG)

#Фото тестової хардварної частини:
-Передавач
![transmiter](https://user-images.githubusercontent.com/74230330/130126956-c40d524a-c8b8-44e8-8a9e-abefb560b3ed.jpg)
-Приймач
![receiver](https://user-images.githubusercontent.com/74230330/130126960-15720213-2d8e-4adb-aeff-bf22fdc17901.jpg)
