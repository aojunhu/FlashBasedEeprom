#ifndef EEPROM_VAR_H
#define EEPROM_VAR_H
  
//������ ������� ������, ���������������� � �������������
#define VOLATILE_EEPROM_SIZE        512
//������� �������������� ������ eeprom
#define TOTAL_EEPROM_SIZE           4096

#define volatile_eeprom_start       0
#define volatile_eeprom_end         volatile_eeprom_start + VOLATILE_EEPROM_SIZE

#define eeprom_cnt                  volatile_eeprom_end + 1  //������� ��� ����������� ��������� ����������� �������
#define eeprom_version              eeprom_cnt + 2  //������� ������� eeprom ����� "EE_VER 002"

///Protected with checksum_eeprom

//������ ���������� checksum_eeprom �������
#define protected_eeprom_start      eeprom_version + EEPROM_VER_LENGTH
#define protected_eeprom_end        (protected_eeprom_start + 100)

//����������� ����� �� protected_eeprom_start - protected_eeprom_end
#define checksum_eeprom             (TOTAL_EEPROM_SIZE - 5)

//����� ����� eeprom
#define eeprom_end                  checksum_eeprom + 1

#endif  //EEPROM_VAR_H


