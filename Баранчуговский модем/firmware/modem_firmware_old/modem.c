#include <avr/eeprom.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
#include "route.h"




/* Обработчик пакетов */

void packet_handler(void) {

    uint8_t temp;

	rfm_22_read_burst(0x47, header , 3);				//считывает заголовок
	rfm_22_write(0x3d,header[0]);						//записывает адрес устройства, отправившего пакет (на случай отправки команды переслать пакет)


	switch (header[1]) {
		case 0x01 : receive_single_data_packet();       // команда одинарный пакет
					if(packet_condition == 0xff) {

						temp = eeprom_read_byte(address_table_eep + *(buffer));

						if (temp == device_address)  rs485_transmit_packet();
						else    if (temp != 0) {
                                   rfm_22_write(0x3d,temp);
                                    transmit_data_packet();
                                    }

						packet_condition = 0x00;		//обнуляет байт состояния пакета
						npack = 0;						//на всякий случай
						}
					break;

		case 0x02 : receive_multiple_data_packet();     // команда пакет из нескольких частей
					if(packet_condition == 0xff) {

					    temp = eeprom_read_byte(address_table_eep + *(buffer));

						if (temp == device_address)  rs485_transmit_packet();
						else    if (temp != 0) {
                                    rfm_22_write(0x3d,temp);
                                    transmit_data_packet();
                                    }

						packet_condition = 0x00;
						npack = 0;						//на всякий случай
						}
					break;

		case 0x03 : rfm_22_write(0x08,0x02);										// стирает fifo
					rfm_22_write(0x08,0x00);

					rfm_22_write(0x3b,0x01);
					rfm_22_transmit_packet(0,npack);
					break;

		case 0x04 : rfm_22_write(0x08,0x02);										// стирает fifo
					rfm_22_write(0x08,0x00);

					rfm_22_write(0x3b,0x02);
					rfm_22_write(0x3c,header[2]);
					if( (header[2]&0x0f) == (header[2]>>4) )
						rfm_22_transmit_packet(((header[2]-1)&0x0f)<<6,npack%64);
					else
						rfm_22_transmit_packet(((header[2]-1)&0x0f)<<6,64);
					break;

		case 0x05 :	npack = 0;
                    packet_condition = 0;
					break;

		case 0x10 : if ((device_type == 1) & (master_address == 0)) {				// проверяет, относится ли к нему эта команда, и привязан ли он уже к сети

                        rfm_22_write(0x3d,header[0]);
						rfm_22_write(0x3a,device_address);
						rfm_22_write(0x3b,0x12);

                        table_reset();                                              // обнуляет таблицу устройств
                        ping_device();                                              // пингует устройства по RS485
                        temp = read_device_list();                                  // записывает адреса устройств в буфер

						_delay_ms(MODEM_RECEIVE_COMMAND_TIME*(device_address&0x07));

						rfm_22_transmit_packet(0,temp);								// отправляет ответ
						}
					 break;

        case 0x11 : if ( (device_type == 2) & (master_address == 0) ) {             // проверяет, относится ли к нему эта команда, и привязан ли он уже к сети

                        ping_device();                                              // пингует устройства по RS485

                        pi

                        }

        case 0x12 : receive_single_data_packet();
                    rfm_22_write(0x3a,device_address);
                    rfm_22_write(0x3b,0x13);

                    update_table(buffer, header[0]);

                    rfm_22_transmit_packet(0,0);
                    break;

		case 0x13 :	if (master_address == 0) {
						master_address = header[0];									// в случае подтверждения записывает master_address
						eeprom_write_byte(&master_address_eep,header[0]);
						}

					 break;
		}
	}





//* Принимает пакет с данными, состоящий из одной части. Проверяет контрольную сумму, если все верно: считывает пакет в буфер модема, заполняет байт состояния пакета как ff, отправляет команду "пакет успешно передан".
 Иначе - отправляе команду переслать пакет. */


void receive_single_data_packet (void) {

	uint8_t lenght;

    lenght = rfm_22_read_packet(0);

	if ( rfm_22_read(0x03) & 0x01 ) {			// проверяет crc

		rfm_22_write(0x3b,0x06);				// если все хреново - отправляет команду переслать пакет
		rfm_22_transmit_small_packet(0,0);

		packet_condition = 0x01;
		}

	else {

		npack = lenght;
		packet_condition = 0xff;

//		rfm_22_write(0x3b,0x05);				// ... и отправляет команду "пакет успешно передан"
//		rfm_22_transmit_packet(0,0);
		}
	}





void receive_multiple_data_packet() {

	uint8_t part, partn, lenght=0;

	part = header[2]>>4;
	partn = header[2] & 0x0f;

	if ( rfm_22_read(0x03) & 0x01 ) {								// проверяет crc

		lenght = rfm_22_read_packet((partn-1)<<6);					// считывает пакет

		packet_condition |= 1<<partn;								// корректирует байт состояния пакета
		}

	if( part == partn ) {
		packet_condition += 0x80;									// если последний пакет - заполняет бит проверки crc
		npack = lenght;
		}

	if( packet_condition > 0x7F ) {									// если проверка crc включена
		if( ((packet_condition & 0x7f) +1) < (1<<part) )  {		// проверяет, есть ли битые пакеты

			partn = part;
			while(packet_condition & (1<<partn)) partn--;			// ищет битый пакет

			rfm_22_write(0x3c,((part<<4)+partn));					// номер битого пакета
			rfm_22_write(0x3b,0x04);								// отправляет команду переслать пакет
			rfm_22_transmit_small_packet(0,0);
			}
		else {
			packet_condition = 0xff;								// состояние пакета - успешно передан
			npack += (part-1)<<6;

			rfm_22_write(0x3b,0x05);								// отправляет команду "пакет успешно передан"
			rfm_22_transmit_small_packet(0,0);
			}
		}
	}




/* Передает пакет данных, если пакет больше 64 байт - делит его на несколько частей и отправляет все его части.
    ВНИМАНИЕ!! Подразумевается что адрес получателя уже записан */

void transmit_data_packet (void) {

	register uint8_t i=0;

	uint8_t part, lastl;

	part=npack>>6;											//вычисляет количество частей пакетов
	lastl=npack&0x3f;										//вычисляет размер последнего
	if( lastl != 0 ) ++part;

	if ( part>1 )	rfm_22_write(0x3b,0x02);				//command id пакет из нескольких частей
	else 			rfm_22_write(0x3c,0x01);				//command id один пакет

	for( i=1 ; i<part ; ++i) {
		rfm_22_write(0x3c,((part<<4)+i));				//записывает номер текущего пакета и их количество
		rfm_22_transmit_small_packet((i-1)<<6,64);			//отправляет пакет
		}

	rfm_22_write(0x3d,((part<<4)+part));
	if( lastl != 0 )	rfm_22_transmit_small_packet(part<<6,lastl);
	else				rfm_22_transmit_small_packet(part<<6,64);

    packet_condition = 0x10;

	}




/* задержка по таймеру */

void delay_timer(uint16_t cycles) {
	++cycles;
	TCNT1=0;
	while(TCNT1 < cycles);
	}




/* Бессмысленная хрень */

/*
void receive_data_packet	(void) {

	register uint8_t i=0;
	uint8_t part, crc, lenght;

	if (header[1]==0x01)	part=1;
	else	part=header[2]>>4;

	npack=lenght;
	crc=(rfm_22_read(0x31) & 0x04);

	for(i=1 ; i<part ; ++i) {
		wait_packet(WAIT_FOR_A_PACK_CYCLES);			// ожидает пакета
		lenght = rfm_22_receive(npack);					// принимает пакет
		npack += lenght;
		crc += ((rfm_22_read(0x31) & 0x04)<<i);			// записывает CRC
		}

	rfm_22_write(0x3c,0x03);								// команда повторной передачи пакета

	while( crc>0 )	{										// пока пакет не будет передан успешно
		for(i=0 ; i<part ; ++i) {							// проверка CRC каждой части пакета
			if ((crc>>i) & 1)	{
				LED_PORT |= RED_LED;						// вкл красный светодиод
				rfm_22_write(0x3d,(((part)<<4)+(i+1)));		// номер битого пакета
				rfm_22_transmit(0,0);						// отправляет команду повторной отправки

				_delay_ms(MODEM_RECEIVE_COMMAND_TIME);

				LED_PORT &= ~RED_LED;						// выкл красный светодиод
				wait_packet(WAIT_FOR_A_PACK_CYCLES);		// ждет ответа
				rfm_22_receive(i*64);						// повторно принимает пакет
				crc -= ~((rfm_22_read(0x31) & 0x04)<<i);	// корректирует переменную CRC
				}
			}
		}

	rfm_22_write(0x3c,0x04);
	rfm_22_write(0x3d,0x00);
	rfm_22_transmit(0,0);									// отправляет подтверждение успешной передачи пакета
	}
*/



/*
void master_transmit (void) {

	register uint8_t i;


	rfm_22_write(0x3a,device_address);


	for ( i=0 ; i<nmodem ; ++i ) {
		rfm_22_write(0x3d, *(modem_address + i));
		transmit_data_packet();

		TCNT1=0;
		sei();										// включает прерывания

		while(TCNT1 < (npack + 30))				// ожидает запросов повторной пересылки пакета
			if(packet_condition == 0xff) break;		// если пакет успено передан - перестает ждать
		cli();
		packet_condition = 0x00;
		}

	for ( i=0 ; i<nretx ; ++i ) {
		rfm_22_write(0x3d, *(retx_address + i));
		transmit_data_packet();

		TCNT1=0;
		sei();										// включает прерывания

		while(TCNT1 < (npack + 30)) 				// ожидает запросов повторной пересылки пакета
			if(packet_condition == 0xff) break;		// если пакет успено передан - перестает ждать
		cli();
		packet_condition = 0x00;

		rfm_22_write(0x3d, master_address);
		}
	}
*/
