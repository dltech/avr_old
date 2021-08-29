#include <avr/io.h>

#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
#include "route.h"




/* Пингует устройтва по радиоканалу */

uint8_t ping_modem_retx(command) {

	rfm_22_write(0x3d,0xff);									// широковещательный адрес
	rfm_22_write(0x3a,device_address);
	rfm_22_write(0x3b,command);

	rfm_22_transmit_packet(0,0);								// отправляет широковещтельный запрос

    }




/* Пингует устройства, подключенные к модему по RS485, после чего ответившие устройства добавляет в таблицу адресов */

void ping_device (void) {

    uint8_t  address_table[255], i, quantity = 0, address;
    uint16_t crc, crcb;

    for(i=0 ; ++i ; i<MODBUS_PACK_LENGHT) *(buffer + i) = 0;        // обнуляет буффер

    for(i=1 ; ++i ; i<=256) {                                       // цикл по всем возможным адресам устройств

        *buffer = i;                                                // адрес устройства
        *(buffer + 1) = 100;                                        // команда ping

        crc = CRC16(buffer,MODBUS_PACK_LENGHT);                     // вычисление контрольной суммы
        *(buffer + MODBUS_PACK_LENGHT - 1) = crc;                   // добавляет контрольную сумму к пакету
        *(buffer + MODBUS_PACK_LENGHT - 2) = (crc & 0xff00) >> 8;

        rs485_transmit_packet();                                    // отправляет пакет

        TCNT1 = 0;
        while (TCNT1 < 2) {                                         // цикл ожидания ответа

            if (UCSRA & 0x80) {                                     // если пакет пришел

                rs485_receive_packet();                             // считывает его

                if (npack != 0) {                                   // если он действительно пришел

                    crc = CRC16(buffer,MODBUS_PACK_LENGHT);
                    crcb = *(buffer + MODBUS_PACK_LENGHT - 1) + ( *(buffer + MODBUS_PACK_LENGHT - 2) << 8 );

                    if  ( (crcb == crc) & (*(buffer + 1) == 100) )  {                                                // если контрольная сумма совпадает и команда верна

                        *(address_table + quantity) = i;                                                             // записывает адрес ответившего устройства в таблицу
                        ++quantity;
                        }

                    for(i=0 ; ++i ; i<MODBUS_PACK_LENGHT) *(buffer + i) = 0;                                         // обнуляет буфер
                    }
                }

            }

        }

    *(address_table + quantity) = 0;

    for(i=0 ; i<quantity ; ++i) {

        *(buffer + i) = *(address_table + i);
        rfm_22_write(0x7f,i);
        }

    update_table(address_table, device_address);

    }




/* обнуление таблицы адресов */

void table_reset (void) {

    register uint8_t i, address;

    for (i=1 ; ++i ; i<256) {
        address = eeprom_read_byte(address_table_eep + i);                 // считывает адрес
        if(address != 0) eeprom_write_byte((address_table_eep + i), 0);    // обнуляет его (если он уже не ноль)
        }
    }




/* Записывает адреса устройств из списка в буфер пакета, передает количество устройств */

uint8_t read_device_list(void) {

    uint8_t i, address, quantity;

    for (i=1 ; ++i ; i<256) {
        address = eeprom_read_byte(address_table + i);                     // считывает адрес
        if(address != 0) {
            *(buffer + quantity) = i;                                      // записывает номер адреса, если он не 0
            rfm_22_write(0x7f, i);
            ++quantity;
            }
        }

    rfm_22_write(0x7f, 0);
    *(buffer + quantity) = 0;

    return quantity;
    }




/* Обновляет таблицу адресов, в качестве аргумента передается список адресов, и адрес устройства, к которому они относятся */

void update_table(*uint8_t address_table, send_address) {

    uint8_t i=0, address;

    while ( *(device_table + i) ) {

        address = eeprom_read_byte(address_table_eep + *(address_table + i));
        if ( address != *(address_table+i) )
            eeprom_write_byte((address_table_eep+*(address_table+i)),send_address);

        ++i;
        }
    }




uint16_t CRC16(uint8_t *pBytes, uint16_t bytesLength)
{
	uint16_t data;
	uint8_t bitsCount, bytesCount;

	data = 0xFFFF;
	for(bytesCount = 0; bytesCount < bytesLength; bytesCount++)
	{
		data ^= pBytes[bytesCount];
		for( bitsCount = 0; bitsCount < 8; bitsCount++ )
		{
			if (data & 0x0001)
			{
				data >>= 1;
				data ^= 0xA001;
			}
			else data >>= 1;
		}
	}

	return data;
}


