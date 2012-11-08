#include  <includes.h>

#include "src\utils\utils_srv.h"
//#include "src\gsm_uart_task\gsm_uart_task.h"
#include "Flash.h"

#include "ee_serv.h"

//������ � eeprom ����� ������. �������� � ����������� ������ ������� � 4 ������
void ee_put(int adr, unsigned char data)
{  
  unsigned char data_buf[1];
  
  CPU_SR_ALLOC();
  
  data_buf[0] = data;
  
  BSP_WatchdogOff();
  
  //��������� ���������� ��� ��������� � ���� ��� ������
  OS_CRITICAL_ENTER();
  FlashCopySectorWithRecovery(FlashCurrentSector(), FlashNextSector(), adr, data_buf, 1);
  OS_CRITICAL_EXIT();
  
  BSP_WatchdogOn();
}

//������ ����� ������ �� eeprom
unsigned char ee_get(int adr)
{
  unsigned char data[1];
  
  CPU_SR_ALLOC();
  
  //��������� ���������� ��� ������ ���� ��� ������ ����������� ������ � ������ Flash.c
  OS_CRITICAL_ENTER();
  FlashRead(FlashCurrentSector(), adr, data, 1);
  OS_CRITICAL_EXIT();
  
  return data[0];
}

//������ � eeprom ������� ������.
void ee_put_ram(int adr, unsigned char *data, int data_size)
{  
  CPU_SR_ALLOC();
    
  BSP_WatchdogOff();
  
  OS_CRITICAL_ENTER();
  FlashCopySectorWithRecovery(FlashCurrentSector(), FlashNextSector(), adr, data, data_size);
  OS_CRITICAL_EXIT();
  
  BSP_WatchdogOn();
}

//������ �� eeprom ������� ������
void ee_get_ram(int adr, unsigned char *data, int data_size)
{
  CPU_SR_ALLOC();
    
  OS_CRITICAL_ENTER();
  FlashRead(FlashCurrentSector(), adr, data, data_size);
  OS_CRITICAL_EXIT();
}

//������ � eeprom 16bit ������. �������� � ����������� ������ ������� � 4 ������
void ee_put_16(int adr, unsigned int data)
{  
  unsigned char data_buf[2];
  
  data_buf[0] = data & 0xFF;
  data_buf[1] = (data >> 8) & 0xFF;
  
  ee_put_ram(adr, data_buf, 2);
}

//������ 16bit ������ �� eeprom
unsigned int ee_get_16(int adr)
{
  unsigned char data[2];
  int value;
  
  ee_get_ram(adr, data, 2);
  
  value = ((data[0] & 0xFF) + ((data[1] & 0xFF) << 8));
  
  return value;
}

//����� eeprom � �������� �� ���������
void ee_reset_to_default()
{
  CPU_SR_ALLOC();
    
  BSP_WatchdogOff();
  
  OS_CRITICAL_ENTER();
  FlashCopySectorFromScratch(FlashCurrentSector(), FlashNextSector());
  OS_CRITICAL_EXIT();
  
  BSP_WatchdogOn();
}




