#ifndef EEPROM_SRV_H
#define EEPROM_SRV_H

/*
������ ������ � eeprom.

���� checksum_eeprom �� ��������� � ����������� ������, ������������ �� protected_eeprom_end ��
protected_eeprom_start, �� ������� ������ �� protected_eeprom_start �� protected_eeprom_end
����������������� �� ��������� �� ���������.

���� eeprom_version �� ��������� � eeprom_version_flash, �� eeprom_version ����������������� ��
eeprom_version_flash, dealer_code �� dealer_code_default. ��������� ������ eeprom �����������������
��� ��� ���� ����������� �����.
*/

#include "eeprom_var.h"

//������ � eeprom ����� ������. �������� � ����������� ������ ������� � 4 ������
void ee_put(int adr, unsigned char data);

//������ ����� ������ �� eeprom
unsigned char ee_get(int adr);

//������ � eeprom ������� ������.
void ee_put_ram(int adr, unsigned char *data, int data_size);

//������ �� eeprom ������� ������
void ee_get_ram(int adr, unsigned char *data, int data_size);

//������ � eeprom 16bit ������. �������� � ����������� ������ ������� � 4 ������
void ee_put_16(int adr, unsigned int data);

//������ 16bit ������ �� eeprom
unsigned int ee_get_16(int adr);

//����� eeprom � �������� �� ���������
void ee_reset_to_default();

#endif  // EEPROM_SRV_H
