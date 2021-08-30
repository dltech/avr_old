#include <avr/io.h>

#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
#include "route.h"


uint8_t ping(uint8_t command) {

	register uint8_t i=0;

	uint8_t nndevice = ndevice;

	rfm_22_write(0x3d,0xff);									// широковещательный адрес
	rfm_22_write(0x3a,device_address);
	rfm_22_write(0x3b,command);

	rfm_22_transmit_packet(0,0);								// отправляет широковещтельный запрос

//	while(!(rfm_22_read(0x02) & 0x04));							// ждет отправления запроса

	TCNT1=0;

	rfm_22_write(0x07,0x05);

	while(TCNT1<REGUEST_WAIT_CYCLES) {

		if (RFM_22_GPIO2_PIN != RFM_22_GPIO2_EN) {

			rfm_22_read_burst(0x47, header, 3);					//считывает заголовок
			if ( header[3] == 0x12)     *(ping_address + nndevice) = header[2];

			for (i=0 ; i<ndevice ; ++i)							// проверяет, есть ли устройство в списке
				if ( *(ping_address+nndevice) == *(ping_address+i) ) --nndevice;

			++nndevice;

			rfm_22_write(0x08,0x03);							//стирает fifo
			rfm_22_write(0x08,0x00);

			rfm_22_write(0x07,0x05);
			TCNT1 = 0;
			}
		}

	rfm_22_write(0x3b,0x13);									// команда привязать к сети
	for(i=ndevice ; i<nndevice ; ++i) {						    // привязывает устройства к подсети
		rfm_22_write(0x3d,*(ping_address+i));
		rfm_22_transmit_packet(0,0);
		delay_timer(20);										// ждет пока дойдет пакет
		}

	ndevice=nndevice;

	led_flash(1,1);

	return ndevice;
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
            if (UCSRA & 0x80) {

                rs485_receive_packet();

                crc = CRC16(buffer,MODBUS_PACK_LENGHT);
                crcb = *(buffer + MODBUS_PACK_LENGHT - 1) + ( *(buffer + MODBUS_PACK_LENGHT - 2) << 8 );

                if  ( (npack != 0) & (crcb == crc) & (*(buffer + 1) == 100) )  {          // если пакет пришел успешно, контрольная сумма совпадает и команда верна

                    *(address_table + quantity) = i;                                      // записывает адрес ответившего устройства в таблицу
                    ++quantity;
                    }

                for(i=0 ; ++i ; i<MODBUS_PACK_LENGHT) *(buffer + i) = 0;                  // обнуляет буффер
                }
            }
        }


    for (i=0 ; i<quantity ; ++i) {                                                        // записывает адреса устройств в eeprom

        address = eeprom_read_byte(address_table_eep + *(address_table + i));
        if ( address != *(address_table+i) )
            eeprom_write_byte((address_table_eep+*(address_table+i)),device_address);

        }

    }




/* обнуление таблицы адресов */

void table_reset (void) {

    register uint8_t i, address;

    for (i=1 ; ++i ; i<256) {
        address = eeprom_read_byte(address_table_eep + i);                 // считывает адрес
        if(address != 0) eeprom_write_byte((address_table_eep + i), 0);    // обнуляет его (если он уже не ноль)
        }
    }



/* Записывает адреса устройств из списка в FIFO RFM */

void read_device_list(void) {

    register uint8_t i, address, a=0;

    for (i=1 ; ++i ; i<256) {
        address = eeprom_read_byte(address_table + i);                     // считывает адрес
        if(address != 0)    rfm_22_write(0x7f,i);                          // записывает номер адреса, если он не 0
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


