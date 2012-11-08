#include <global.h>

#ifndef __FLASH_H_
#define __FLASH_H_

/*
������ �������� eeprom �� ���� ����������� � ��������� flash. Flash ��� LPC1768
��������� ������� �� 512 ����� ������, �� ����� ������� ��������� ������ ������� �������
Flash. ��� LPC1768 ������ 16 �������� �� 4 �����, � ��������� 14 �������� �� 32 �����.

����� ����, � ������� ������� Flash �������� ������� ���������� � �������������� ��� ������
��� ������������� ������ CRP.

����� ������ � Flash ���������:
1. � �������� eeprom ����� ������������� ������� 1 ������ � 4 �����.
2. ��� ����������� eeprom ������������� N �������� �� EE_FIRST_SECTOR �� EE_LAST_SECTOR
3. ��� ������ ������ � eeprom ������� ������ ���������� � ��������� �� ������� (���������
   ������ ��� ���� ���������).
4. �� ����� ����������� ������������ ������ ������, ����������� ����������� ����� �
   ����������� ������� ��������.
5. ��� ������ ��������� �� �������� �������� ������������ ����� ����� ������. �����
   �������������� ������������ ��������.
6. ��� ���� ����������� ����� ����������������� ���������� ����� ����� ������. ���� ���
   �� ������ ��������� �������, eeprom �������� �� ���������.
7. ��� ����������� �������� ������������ ����� �������� EE_BLOCK_SIZE. EE_BLOCK_SIZE - ���
   ���������� ������ ��� ������ �������.
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

//������������� IAP. ����� ������ ������ ������� ��� ������
//
void FlashInit();

//���������� ������� ������ ��� �����
int FlashCurrentSector();

//���������� ��������� ������ ��� �����
int FlashNextSector();

//���������� ��������� ������ ��� ����� ��� ���������
int FlashNextSectorFor(int sector);

//�������� �������
BOOL FlashErase(int sector);

//�������� ������� �� �������
//���������� TRUE, ���� ������ �� ����
BOOL FlashBlankCheck(int sector);

//������ �� ����, ������� ����� sector, �������� �� ������ ������� offset,
//� ����� buffer_out, count ����
//��������� �� ����� EE_BLOCK_SIZE �� ���
void FlashRead(int sector, int offset, unsigned char *buffer_out, int count);

//������ ������ �� ����
BOOL FlashWrite(int sector, int offset, int count, unsigned char *buffer);

//����������� ������� ������� sector_start � sector_end. sector_end
//����� ����
BOOL FlashCopySector(int sector_start, int sector_end);

//�������� ������ �� block_start � block_end, ������� ������, ������� � ������ where_to_replace ������� data
//��������� data_size ����
BOOL FlashCopySectorAndReplaceData(int sector_start, int sector_end, int where_to_replace, unsigned char *data, int data_size);

//��������� FlashCopySectorAndReplaceData.
//���� ������������ DEF_FALSE ��� �������� ��� ������ ������� �������, �� ��������
//����������� ��� �� ��������� ������ � ���, ���� �� ��������� ��� �������
BOOL FlashCopySectorWithRecovery(int sector_start, int sector_end, int where_to_replace, unsigned char *data, int data_size);

//����������� ������� ������� sector_start � sector_end. sector_end
//����� ����. ���������� ������ �� ��������� (����� ������).
BOOL FlashCopySectorFromScratch(int sector_start, int sector_end);

#endif //__FLASH_H_
