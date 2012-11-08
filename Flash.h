#include <global.h>

#ifndef __FLASH_H_
#define __FLASH_H_

/*
Модуль эмуляции eeprom на базе встроенного в процессор flash. Flash для LPC1768
позволяет хранить до 512 Кбайт данных, но перед записью требуется полная очистка сектора
Flash. Для LPC1768 первые 16 секторов по 4 Кбайт, а следующие 14 секторов по 32 Кбайт.

Кроме того, в нулевом секторе Flash хранятся вектора прерываний и перезаписывать его нельзя
при установленной защите CRP.

Схема работы с Flash следующая:
1. В качестве eeprom может резервируется целиком 1 сектор в 4 Кбайт.
2. Для организации eeprom резервируется N секторов от EE_FIRST_SECTOR до EE_LAST_SECTOR
3. При записи данных в eeprom текущий сектор копируется в следующий по порядку (следующий
   сектор при этом стирается).
4. Во время копирования дописываются нужные данные, формируется контрольная сумма и
   добавляется счётчик секторов.
5. При старте программы по счётчику секторов определяется самый новый сектор. Также
   обрабатывается переполнение счётчика.
6. При сбое контрольной суммы восстанавливается предыдущий самый новый сектор. Если нет
   ни одного валидного сектора, eeprom создаётся по умолчанию.
7. При копировании секторов используется буфер размером EE_BLOCK_SIZE. EE_BLOCK_SIZE - это
   допустимый размер при записи сектора.
*/

#define CCLK    CPU_MAIN_CLOCK

#define kIAPentry 0x1FFF1FF1

#define EE_BLOCK_SIZE       256
#define EE_SECTOR_SIZE      4096
#define EE_FIRST_SECTOR     1
#define EE_LAST_SECTOR      14

//EE_MAX_CNT - EE_MIN_CNT should be greater than EE_LAST_SECTOR - EE_FIRST_SECTOR
#define EE_INVALID_ZERO     0x0000
#define EE_INVALID_ONES     0xFFFF
#define EE_MIN_CNT          0x0001
#define EE_MAX_CNT          0x7FFE


enum {
  CMD_PREPARE_SECTORS = 50,
  CMD_COPY_RAM_TO_FLASH,
  CMD_ERASE_SECTORS,
  CMD_BLANK_CHECK_SECTORS,
  CMD_READ_PART_ID,
  CMD_READ_BOOT_CODE_VERSION,
  CMD_COMPARE
};

enum {
  STATUS_CMD_SUCCESS = 0,
  STATUS_INVALID_COMMAND,
  STATUS_SRC_ADDR_ERROR,
  STATUS_DST_ADDR_ERROR,
  STATUS_SRC_ADDR_NOT_MAPPED,
  STATUS_DST_ADDR_NOT_MAPPED,
  STATUS_COUNT_ERROR,
  STATUS_INVALID_SECTOR,
  STATUS_SECTOR_NOT_BLANK,
  STATUS_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
  STATUS_COMPARE_ERROR,
  STATUS_BUSY
};

typedef void (__thumb *IAP)(void*, void*);

//инициализация IAP. Поиск самого нового сектора для записи
//
void FlashInit();

//возвращает текущий сектор для флэша
int FlashCurrentSector();

//возвращает следующий сектор для флэша
int FlashNextSector();

//возвращает следующий сектор для флэша для заданного
int FlashNextSectorFor(int sector);

//стирание сектора
BOOL FlashErase(int sector);

//проверка сектора на пустоту
//возвращает TRUE, если сектор не пуст
BOOL FlashBlankCheck(int sector);

//Чтение из флэш, сектора номер sector, смещение от начала сектора offset,
//в буфер buffer_out, count байт
//Считывать не более EE_BLOCK_SIZE за раз
void FlashRead(int sector, int offset, unsigned char *buffer_out, int count);

//Запись буфера во флэш
BOOL FlashWrite(int sector, int offset, int count, unsigned char *buffer);

//копирование целиком сектора sector_start в sector_end. sector_end
//будет стёрт
BOOL FlashCopySector(int sector_start, int sector_end);

//копирует сектор из block_start в block_end, заменяя данные, начиная с адреса where_to_replace данными data
//записывая data_size байт
BOOL FlashCopySectorAndReplaceData(int sector_start, int sector_end, int where_to_replace, unsigned char *data, int data_size);

//выполняем FlashCopySectorAndReplaceData.
//Если возвращается DEF_FALSE или чексумма для нового сектора неверна, то пытаемся
//скопировать его на следующий сектор и так, пока не переберем все сектора
BOOL FlashCopySectorWithRecovery(int sector_start, int sector_end, int where_to_replace, unsigned char *data, int data_size);

//копирование целиком сектора sector_start в sector_end. sector_end
//будет стёрт. Записывает данные по умолчанию (кроме ключей).
BOOL FlashCopySectorFromScratch(int sector_start, int sector_end);

#endif //__FLASH_H_
