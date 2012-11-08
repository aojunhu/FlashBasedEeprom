#ifndef EEPROM_SRV_H
#define EEPROM_SRV_H

/*
Модуль работы с eeprom.

Если checksum_eeprom не совпадает с контрольной суммой, рассчитанной от protected_eeprom_end до
protected_eeprom_start, то область памяти от protected_eeprom_start до protected_eeprom_end
восстанавливается на настройки по умолчанию.

Если eeprom_version не совпадает с eeprom_version_flash, то eeprom_version восстанавливается на
eeprom_version_flash, dealer_code на dealer_code_default. Остальная память eeprom восстанавливается
как при сбое контрольной суммы.
*/

#include "eeprom_var.h"

//Запись в eeprom байта данных. Пирводит к перемещению целого сектора в 4 КБайта
void ee_put(int adr, unsigned char data);

//Чтение байта данных из eeprom
unsigned char ee_get(int adr);

//запись в eeprom массива данных.
void ee_put_ram(int adr, unsigned char *data, int data_size);

//чтение из eeprom массива данных
void ee_get_ram(int adr, unsigned char *data, int data_size);

//Запись в eeprom 16bit данных. Приводит к перемещению целого сектора в 4 КБайта
void ee_put_16(int adr, unsigned int data);

//Чтение 16bit данных из eeprom
unsigned int ee_get_16(int adr);

//сброс eeprom в значение по умолчанию
void ee_reset_to_default();

#endif  // EEPROM_SRV_H
