# STM32 RFM69 HAL Library
Проєкт бібліотеки радіомодуля RFM69 на С, для мікроконтролерів STM32. Використовує HAL.
Протестовано на STM32F051R8 та STM32F334R8.

# Установка
1) Тягнемо у проєкт файли: 
-rfm69_registers.h
-rfm69.h
-rfm69.с
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
b) Для приймача, оголошуємо елемент структури Payload і ресівимо дані в її поля.
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
Парочка фоток процесу:
1) Трансмітер на STM32F030.
![photo_2021-05-20_23-15-57](https://user-images.githubusercontent.com/74230330/119043283-8bc98a80-b9c1-11eb-9ceb-1076bc3d625c.jpg)
Живиться від павербанку, для того щоб тестувати дальність передачі
![photo_2021-05-20_23-16-26](https://user-images.githubusercontent.com/74230330/119043272-89ffc700-b9c1-11eb-9cdb-25a997d5d83e.jpg)
2)  Ресівер на STM32F051, з UART у який він випльовує прийняті дані.
![photo_2021-05-20_23-15-57 (2)](https://user-images.githubusercontent.com/74230330/119043282-8b30f400-b9c1-11eb-863a-0d6382a9d1b8.jpg)
3) Призначення виводів STM32
![Снимок](https://user-images.githubusercontent.com/74230330/128074874-38a1fd05-f855-40c2-ac01-00b0569efd6f.JPG)
5) Налаштування SPI

![SPI_Settings](https://user-images.githubusercontent.com/74230330/128081021-bec453f8-cec2-41f5-8124-864e07bdbdc8.JPG)

6) upd 19.08.2021
#Оновлений хардвар:
-Передавач
![transmiter](https://user-images.githubusercontent.com/74230330/130126956-c40d524a-c8b8-44e8-8a9e-abefb560b3ed.jpg)
-Приймач
![receiver](https://user-images.githubusercontent.com/74230330/130126960-15720213-2d8e-4adb-aeff-bf22fdc17901.jpg)
