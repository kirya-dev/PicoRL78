## Программный инструмент для чтения защищенных микропроцессоров Renesas RL78.
- Реализуемая задача: Получение полного дампа **DFLASH** и **CFLASH** через отладку(OCD) с предварительным обходом защит.

Необходимые инструменты:
- RP2040;
- Прошить PicoRL78.ino в среде Arduino IDE предварительно установив Board [Rapsberry Pi Pico](https://github.com/earlephilhower/arduino-pico?tab=readme-ov-file#installation);
- MOSFET транзистор IRLML2030, IRLML2502 или любой быстрый N-канальный;
- MCU_GLITCH_PIN Подключить к затвору MOSFET;
- Сток MOSFET к VDD или REGC (снять емкости с обоих линий если имеются);
- Исток MOSFET к GND;
- Выводы RX TX (пины 0 и 1) объединить и соединить с TOOL0 микросхемы RL78.


### Замечание
Скрипт читает обе области FLASH сразу при подаче питания с автоматическим обходом защит. Дампы выводитяся в HEX виде в серийный порт на скорости 9600 бод.
Скрипт реализован с обходом нескольких защит одновременно, но пока не проверен.


#### Основано на проекте взлома PS4-syscon
- [пост](https://fail0verflow.com/blog/2018/ps4-syscon/)
- [github](https://github.com/VV1LD/SYSGLITCH/blob/master/TEENSY_2.0%2B%2B/SYSGLITCH.ino#L123-L139)


#### Документация RL78:
- [Protocol A](https://www.renesas.com/en/document/apn/rl78-microcontrollers-rl78-protocol-programmer-edition-application-note)
- [RL78 Microcontroller (RL78 Protocol B) Serial Programming Guide](https://www.renesas.com/en/document/apn/rl78-family-rl78-microcontroller-rl78-protocol-b-serial-programming-guide)
- [RL78/ F13, F14 User’s Manual: Hardware](https://www.renesas.com/en/document/mah/rl78f13-f14-users-manual-hardware-rev220)

#### Взломанные устройства:
- RL78/F13 R5F10AGG - VW 5Q1953569C (защита ID-CODE)
- RL78/F14 R5F10PPJ - Honda Relay Module 38830-TVA-A01 (защита ProtectError)


![HELL](https://github.com/VV1LD/SYSGLITCH/blob/master/hell.jpg)
