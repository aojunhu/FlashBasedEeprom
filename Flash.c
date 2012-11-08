#include <stdio.h>
#include <string.h>

#include <includes.h>

#include "Flash.h"

#include "src\eeprom_srv\ee_serv.h"
#include "src\gsm_uart_task\gsm_uart_task.h"
#include "src\utils\utils_srv.h"

//-------------------------------------------- defines

typedef struct SLayout {
  int sectors;
  int size;
} Layout;

//==============================================
//==============================================
//                32k flash, CortexM3
// LPC1751
//==============================================
//==============================================
//                64k flash, CortexM3
// LPC1752
//==============================================
//==============================================
//                128k flash, CortexM3
// LPC1754
// LPC1764
//==============================================
//==============================================
//                256k flash, CortexM3
// LPC1756
// LPC1765
// LPC1766
//==============================================
//==============================================
//                512k flash, CortexM3
// LPC1758
// LPC1768
//==============================================
//==============================================
//                32k flash, CortexM3/M0
// LPC1x13
// LPC1343
//==============================================
//==============================================
//                16k flash, CortexM3
// LPC1342
//==============================================
//==============================================
//                8k flash, CortexM3/M0
// LPC1x11
//==============================================

//-------------------------------------------- data

const Layout flashLayout[] =
{
  {16,  4096},
  {14, 32768},
  { 0,     0}
};

const int allowedWriteSizes[] =
{
  4096,
  1024,
  512,
  256,
  0
};

static unsigned char ee_buffer[EE_BLOCK_SIZE];
static int current_sector;
static int current_cnt;
static unsigned char current_crc;

//-------------------------------------------- prototypes

/** internal functions **/
static int ExecuteCommand(unsigned long* cmd, unsigned long* status);
static void FlashAddEepromData(uint32_t block_where_address);
static BOOL FlashCheckCrc(int sector);
/** public data **/

/** private data **/
static const IAP iap_entry = (IAP)kIAPentry; // MCU flash firmware interface function.
// The CPU clock speed (CCLK), the default value is used if no clock option is found.

//-------------------------------------------- modules

static int ExecuteCommand(unsigned long* cmd, unsigned long* status)
{
  int ret;

  for (;;)
  {
    iap_entry(cmd, status);
    ret = status[0];
    if (ret != STATUS_BUSY)
    {
      return ret;
    }
    // Try again if busy.
  }
}

//������������� IAP. ����� ������ ������ ������� ��� ������
//� ������ ������� ������� �������� ����� ������������ ���������� ����������� ������. �������� ������
//������ ����� ���������, ������� �������� � ����������� �����.
//���� ��� �� ������ ��������� �������, �� ������ ����� ������������� �� ������� ������� � ���
//��������� �� protected_eeprom ����� ����������� �� ���������
void FlashInit()
{    
  int current_sector_i;
  int current_sector_f;
  int i;

  INT16U ee_cnt_int;
  INT16U min_ee_cnt;
  INT16U current_ee_cnt;
  int min_sector_num;
  BOOL has_valid_sectors;
  unsigned char ee_cnt[2];
  unsigned char ee_version[EEPROM_VER_LENGTH];
  
  has_valid_sectors = DEF_FALSE;
  min_ee_cnt = EE_MAX_CNT;
  
  //�������� �� ���� �������� � ������� ��������� � ������ ������ �������
  //���� ���� ������������ ������� �� ���������� EE_MIN_CNT � EE_MAX_CNT, �� �������, ���
  //����� ����� ������ - ��� �� EE_MIN_CNT �� ������������� � ����� 1. ���� ��� ���������, ��
  //������ �� �������.
  for(current_sector_i = EE_FIRST_SECTOR; current_sector_i <= EE_LAST_SECTOR; current_sector_i++)
  {
    FlashRead(current_sector_i, eeprom_version, ee_version, EEPROM_VER_LENGTH);
    
    if(strncmp((char const *)ee_version, (char const *)eeprom_version_flash, EEPROM_VER_LENGTH) != 0)
      continue;
    
    if(!FlashCheckCrc(current_sector_i))
      continue;
    
    FlashRead(current_sector_i, eeprom_cnt, ee_cnt, 2);
    
    ee_cnt_int = ((ee_cnt[0] & 0xFF) + ((ee_cnt[1] & 0xFF) << 8));
    
    //invalid flash sector
    if((ee_cnt_int == EE_INVALID_ZERO) || (ee_cnt_int == EE_INVALID_ONES))
      continue;
    
    has_valid_sectors = DEF_TRUE;
    
    if(ee_cnt_int <= min_ee_cnt)
    {
      min_ee_cnt = ee_cnt_int;
      min_sector_num = current_sector_i;
    }
  }
  
  //��� �������� �������� - ���������� �� ������
  if(!has_valid_sectors)
  {  
    current_sector = EE_FIRST_SECTOR;
    current_cnt = EE_MIN_CNT;
    
    FlashCopySectorFromScratch(current_sector, FlashNextSector());
    
    return;
  }
  
  //���� �������� �������
  
  //������� ��� �������� �������, ������� � ������� � ����������� ���������
  //���� ��������� ������ ����� ������� ������ ���������� ��� ����� ������� �� 2
  //������� �����������, �� ������ �� ���������� ����� ����� ������ ����
  current_sector_i = min_sector_num;
  current_sector_f = min_sector_num;
  current_ee_cnt = min_ee_cnt;
    
  for(i = EE_FIRST_SECTOR; i <= EE_LAST_SECTOR; i++)
  {    
    FlashRead(current_sector_i, eeprom_version, ee_version, EEPROM_VER_LENGTH);
    
    if(strncmp((char const *)ee_version, (char const *)eeprom_version_flash, EEPROM_VER_LENGTH) != 0)
    {
      if(current_sector_i >= EE_LAST_SECTOR)
      {
        current_sector_i = EE_FIRST_SECTOR;
      }
      else
      {
        current_sector_i++;
      }
      continue;
    }
    
    if(!FlashCheckCrc(current_sector_i))
      continue;
    
    FlashRead(current_sector_i, eeprom_cnt, ee_cnt, 2);
    
    ee_cnt_int = ((ee_cnt[0] & 0xFF) + ((ee_cnt[1] & 0xFF) << 8));
    
    //invalid flash sector
    if((ee_cnt_int == EE_INVALID_ZERO) || (ee_cnt_int == EE_INVALID_ONES))
    {
      if(current_sector_i >= EE_LAST_SECTOR)
      {
        current_sector_i = EE_FIRST_SECTOR;
      }
      else
      {
        current_sector_i++;
      }
      continue;
    }
    
    if(ee_cnt_int < current_ee_cnt)
      break;
    if((ee_cnt_int - current_ee_cnt) > 1)
      break;
    
    current_ee_cnt = ee_cnt_int;
    current_sector_f = current_sector_i;
    
    if(current_sector_i >= EE_LAST_SECTOR)
    {
      current_sector_i = EE_FIRST_SECTOR;
    }
    else
    {
      current_sector_i++;
    }
  }

  //��������� ������ ������ �� ����
  current_sector = current_sector_f;
  current_cnt = current_ee_cnt;
}

//���������� ������� ������ ��� �����
int FlashCurrentSector()
{
  return current_sector;
}

//���������� ��������� ������ ��� �����
int FlashNextSector()
{
  int next_sector;
  
  if(current_sector >= EE_LAST_SECTOR)
    next_sector = EE_FIRST_SECTOR;
  else
    next_sector = current_sector + 1;
  
  return next_sector;
}

//���������� ��������� ������ ��� ����� ��� ���������
int FlashNextSectorFor(int sector)
{
  int next_sector;
  
  if(sector >= EE_LAST_SECTOR)
    next_sector = EE_FIRST_SECTOR;
  else
    next_sector = sector + 1;
  
  return next_sector;
}

//���������� ������� ������� �������� ��� �����
INT16U FlashCurrentCounter()
{
  return current_cnt;
}

//���������� ��������� ������� �������� ��� �����
INT16U FlashNextCounter()
{
  INT16U next_cnt;
  
  if(current_cnt >= EE_MAX_CNT)
    next_cnt = EE_MIN_CNT;
  else
    next_cnt = current_cnt + 1;
  
  return next_cnt;
}

//������ ������ �� ����
BOOL FlashWrite(int sector, int offset, int count, unsigned char *buffer)
{
  int ret;
  unsigned long cmd[5];
  unsigned long status[3];

  cmd[0] = CMD_PREPARE_SECTORS;
  cmd[1] = sector;
  cmd[2] = sector;

  ret = ExecuteCommand(cmd, status);

  if (ret != STATUS_CMD_SUCCESS)
  {
    //message CMD_PREPARE_SECTORS failed.
    gsm_uart_printf_unsafe("FlashWrite prepare error\r");
    return DEF_FALSE;
  }

  cmd[0] = CMD_COPY_RAM_TO_FLASH;
  cmd[1] = (sector * EE_SECTOR_SIZE) + offset;
  cmd[2] = (unsigned long)buffer;
  cmd[3] = count;
  cmd[4] = CCLK/1000;

  ret = ExecuteCommand(cmd, status);

  if (ret != STATUS_CMD_SUCCESS)
  {
    //message CMD_COPY_RAM_TO_FLASH failed.
    gsm_uart_printf_unsafe("FlashWrite copy error\r");
    return DEF_FALSE;
  }

  return DEF_TRUE;
}

//�������� �������
BOOL FlashErase(int sector)
{
  int ret;
  unsigned long cmd[4];
  unsigned long status[3];

  // Prepare sector for erase.
  cmd[0] = CMD_PREPARE_SECTORS;
  cmd[1] = sector;
  cmd[2] = sector;
  
  ret = ExecuteCommand(cmd, status);

  if (ret != STATUS_CMD_SUCCESS)
  {
    //message CMD_PREPARE_SECTORS failed.
    gsm_uart_printf_unsafe("FlashErase prepare error\r");
    return DEF_FALSE;
  }

  // Erase sector.
  cmd[0] = CMD_ERASE_SECTORS;
  cmd[1] = sector;
  cmd[2] = sector;
  cmd[3] = CCLK/1000;
  
  ret = ExecuteCommand(cmd, status);

  if (ret != STATUS_CMD_SUCCESS)
  {
    //message CMD_ERASE_SECTORS failed.
    gsm_uart_printf_unsafe("FlashErase erase error\r");
    return DEF_FALSE;
  }

  return DEF_TRUE;
}

//�������� ������� �� �������
//���������� TRUE, ���� ������ �� ����
BOOL FlashBlankCheck(int sector)
{
  int ret;
  unsigned long cmd[4];
  unsigned long status[3];

  // Prepare sector for erase.
  cmd[0] = CMD_BLANK_CHECK_SECTORS;
  cmd[1] = sector;
  cmd[2] = sector;

  ret = ExecuteCommand(cmd, status);

  if (ret == STATUS_SECTOR_NOT_BLANK)
  {
    return DEF_TRUE;
  }

  return DEF_FALSE;
}

//��������������� ��� ��� ������ � ������ ���� ������
void CheckAndWrite(uint32_t block_where_address, int data_addr, unsigned char *data, int data_size)
{
  int i;

  if(((block_where_address + EE_BLOCK_SIZE) > data_addr) && (block_where_address <= data_addr))
  {
    i = (data_addr - block_where_address) % EE_BLOCK_SIZE;

    memcpy(ee_buffer + i, data, data_size);
  }
}

//��������������� ��� ��� ������ � ������ ���� ������
void CheckAndWriteByte(uint32_t block_where_address, int data_addr, unsigned char data)
{
  int i;

  if(((block_where_address + EE_BLOCK_SIZE) > data_addr) && (block_where_address <= data_addr))
  {
    i = (data_addr - block_where_address) % EE_BLOCK_SIZE;

    ee_buffer[i] = data;
  }
}

#ifdef AUTONOME_MODE
  //���������� ������ � ��������� ������
  static void eeprom_fill_shlf(uint32_t block_where_address, unsigned char num)
  {
    int ee_ptr;
    unsigned const char *fl_ptr;
  
    ee_ptr = AUTO_SHLF_LIST_BODY + AUTO_SHLF_DESCRIPTION_LEN * num;
    fl_ptr = msg_alarm;
  
    while(*fl_ptr)
    {
      CheckAndWriteByte(block_where_address, ee_ptr, *fl_ptr);
      ee_ptr++;
      fl_ptr++;
    }
    CheckAndWriteByte(block_where_address, ee_ptr++, num + 1 + '0');
    CheckAndWriteByte(block_where_address, ee_ptr, 0);
  }
#endif  //#ifdef AUTONOME_MODE

//���������� ������������ ������ ��� ������ ����� (����� ����� ��������)
//������������ ���������� ����� ee_buffer
static void FlashAddScratchData(uint32_t block_where_address)
{
  int i;
  unsigned char buffer[20];
  
  CheckAndWrite(block_where_address, eeprom_version, (unsigned char *)eeprom_version_flash, EEPROM_VER_LENGTH);
  
  //����� MAC
  memcpy(buffer, mac_default, 6);
  CheckAndWrite(block_where_address, EE_MAC_ADDRESS, buffer, 6);
  
  //������������ ������ - ����������� � �����
  FlashAddEepromData(block_where_address);
}

//����������� ������� ������� sector_start � sector_end. sector_end
//����� ����. ���������� ������ �� ��������� (����� ������).
BOOL FlashCopySectorFromScratch(int sector_start, int sector_end)
{
  BOOL ret;
  uint32_t block_from_address;
  uint32_t block_where_address;
  
  block_from_address = sector_start * EE_SECTOR_SIZE;
  block_where_address = 0;
  current_crc = 0;
  
  //�������� �������
  ret = FlashBlankCheck(sector_end);
  
  if(ret)
  {
    if(!FlashErase(sector_end))
    {
      gsm_uart_printf_unsafe("FlashCopySectorFromScratch: cannot erase\r");

      return DEF_FALSE;
    }
  }
  
  while(1)
  {  
    memcpy(ee_buffer, (void *)block_from_address, EE_BLOCK_SIZE);

    FlashAddScratchData(block_where_address);
  
    ret = FlashWrite(sector_end, block_where_address, EE_BLOCK_SIZE, ee_buffer);
  
    if(!ret)
    {
      gsm_uart_printf_unsafe("FlashCopySectorFromScratch: cannot write\r");
      return DEF_FALSE;
    }

    block_from_address += EE_BLOCK_SIZE;
    block_where_address += EE_BLOCK_SIZE;
  
    if(block_where_address >= EE_SECTOR_SIZE)
    {      
      //������� ������� �������� ���, ���� �� ��������� ������      
      current_sector = sector_end;
      current_cnt = FlashNextCounter();
      return DEF_TRUE;
    }
  }
}

//���������� ������������ ������
//������������ ���������� ����� ee_buffer
static void FlashAddEepromData(uint32_t block_where_address)
{
  INT16U next_cnt;
  int i, k;
  
  next_cnt = FlashNextCounter();
  
  //����� ��������� eeprom 
  //CheckAndWrite(block_where_address, eeprom_version, (unsigned char *)eeprom_version_flash, EEPROM_VER_LENGTH);
  
  //������������� ������� ��������  
  CheckAndWriteByte(block_where_address, eeprom_cnt, next_cnt & 0xFF);
  CheckAndWriteByte(block_where_address, eeprom_cnt + 1, (next_cnt >> 8) & 0xFF);
  
  //������������� CRC
  
  //������� CRC ��� ������� �����
  if(((block_where_address + EE_BLOCK_SIZE) > volatile_eeprom_start) && (block_where_address <= volatile_eeprom_start))
  {
    //i - ���������� �� ������ ����� �� protected_eeprom_start
    i = (volatile_eeprom_start - block_where_address) % EE_BLOCK_SIZE;
    //k - ������ protected_eeprom
    k = EE_BLOCK_SIZE - i;
    
    //� ������, ���� protected_eeprom ������������� ������, ��� ���������� ��������� ����, ��
    //������������ ������� CRC �� protected_eeprom_start �� protected_eeprom_end
    if((block_where_address + EE_BLOCK_SIZE) > protected_eeprom_end)
    {
      k = protected_eeprom_end - volatile_eeprom_start;
    }
    
    current_crc = utils_calc_crc(ee_buffer + i, k);
  }
  
  //�� ���������� ����� ������� CRC �� protected_eeprom_end
  if((block_where_address > volatile_eeprom_start) && (block_where_address < protected_eeprom_end))
  {
    //i - ���������� �� ������ ����� �� protected_eeprom_end
    if((protected_eeprom_end - block_where_address) >= EE_BLOCK_SIZE)
    {
      i = EE_BLOCK_SIZE;
    }
    else
    {
      i = (protected_eeprom_end - block_where_address) % EE_BLOCK_SIZE;
    }
    
    for(k = 0; k < i; k++)
    {
      current_crc = utils_calc_crc_feed(ee_buffer[k], current_crc);
    }
  }
    
  //� ��������� � ����, � ������ �������������
  CheckAndWriteByte(block_where_address, checksum_eeprom, current_crc);
}

//�������� ������ �� block_start � block_end, ������� ������, ������� � ������ where_to_replace ������� data
//��������� data_size ����
BOOL FlashCopySectorAndReplaceData(int sector_start, int sector_end, int where_to_replace, unsigned char *data, int data_size)
{
  BOOL ret;
  int i;
  uint32_t block_from_address;
  uint32_t block_where_address;
  
  block_from_address = sector_start * EE_SECTOR_SIZE;
  block_where_address = 0;
  current_crc = 0;
  
  //�������� �������
  ret = FlashBlankCheck(sector_end);
  
  if(ret)
  {
    if(!FlashErase(sector_end))
    {
      gsm_uart_printf_unsafe("FlashCopySectorAndReplaceData: cannot erase\r");

      return DEF_FALSE;
    }
  }
  
  while(1)
  {  
    memcpy(ee_buffer, (void *)block_from_address, EE_BLOCK_SIZE);
    
    //���� ��������� ���� ���������� � ������ ����� ������ where_to_replace, �� �����
    //�������� ������ ��� � ���� �����
    if(((block_where_address + EE_BLOCK_SIZE) > where_to_replace) && (block_where_address <= where_to_replace) && data_size)
    {
      i = (where_to_replace - block_where_address) % EE_BLOCK_SIZE;
      while((i < EE_BLOCK_SIZE) && data_size)
      {
        ee_buffer[i] = *data;
        data_size--;
        data++;
        where_to_replace++;
        i++;
      }
    }

    FlashAddEepromData(block_where_address);
  
    ret = FlashWrite(sector_end, block_where_address, EE_BLOCK_SIZE, ee_buffer);
  
    if(!ret)
    {
      gsm_uart_printf_unsafe("FlashCopySectorAndReplaceData: cannot write\r");
      return DEF_FALSE;
    }

    block_from_address += EE_BLOCK_SIZE;
    block_where_address += EE_BLOCK_SIZE;
  
    if(block_where_address >= EE_SECTOR_SIZE)
    {      
      //������� ������� �������� ���, ���� �� ��������� ������
      current_sector = sector_end;
      current_cnt = FlashNextCounter();
      return DEF_TRUE;
    }
  }
}

//��������� FlashCopySectorAndReplaceData.
//���� ������������ DEF_FALSE ��� �������� ��� ������ ������� �������, �� ��������
//����������� ��� �� ��������� ������ � ���, ���� �� ��������� ��� �������
BOOL FlashCopySectorWithRecovery(int sector_start, int sector_end, int where_to_replace, unsigned char *data, int data_size)
{
  BOOL result;
  int sector_to;
  int i;
  
  sector_to = sector_end;
  i = EE_FIRST_SECTOR;
  
  while(1)
  {    
    result = FlashCopySectorAndReplaceData(sector_start, sector_to, where_to_replace, data, data_size);
    
    if(result && FlashCheckCrc(sector_to))
      return DEF_TRUE;
    
    if(i >= EE_LAST_SECTOR)
      return DEF_FALSE;
    
    sector_to = FlashNextSectorFor(sector_to);
    i++;
  }
}

//������ �� ����, ������� ����� sector, �������� �� ������ ������� offset,
//� ����� buffer_out, count ����
//��������� �� ����� EE_BLOCK_SIZE �� ���
void FlashRead(int sector, int offset, unsigned char *buffer_out, int count)
{
  uint32_t block_from_address;
  
  block_from_address = sector * EE_SECTOR_SIZE;
  
  if(count > EE_BLOCK_SIZE)
    count = EE_BLOCK_SIZE;
  
  memcpy(buffer_out, (void *)(block_from_address + offset), count);
}

//�������� �������� ��� ������� sector.
//���� ��� ����� ����������� �������� �� ������ checksum_eeprom, �� ���������� DEF_TRUE
BOOL FlashCheckCrc(int sector)
{
  int begin, end;
  uint32_t block_from_address;
  int offset;
  int length;
  unsigned char crc_read;
  unsigned char crc_calc;
  
  block_from_address = sector * EE_SECTOR_SIZE;
  offset = checksum_eeprom;
  
  crc_read = *((unsigned char *)(block_from_address + offset));
  
  begin = protected_eeprom_end;
  end = volatile_eeprom_start;
  offset = volatile_eeprom_start;
  
  length = (begin - end);
  
  crc_calc = utils_calc_crc((unsigned char *)(block_from_address + offset), length);
                              
  return (crc_calc == crc_read);
}
