# STM32F0_RFM69_Cpp_rx-tx
Проєкт C++ STM32 CMSIS бібліотеки для роботи  з радіомодулями RFM69W
Початково шматок коду, що працює з RFM було взято у цього товариша: https://github.com/ahessling/RFM69-STM32
Переписано, низькорівневі ф-ї роботи з RCC, GPIO, UART, SPI. Зокрема використано мої C++ CMSIS ліби. В цьому вигляді заведено демо проєкт.
Далі планується переписати/дописати лібу під свої потреби.

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
