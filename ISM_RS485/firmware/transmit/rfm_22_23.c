#include <avr/io.h>
#include <util/delay.h>
#include "rfm_22_23.h"
#include <stdlib.h>




void  rfm_22_read_burst  (uint8_t address, uint8_t *data, uint8_t lenght) {

	register uint8_t i=0;

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address;		 			// записывает адрес
	while (!(SPSR<<7));

	for(i=0 ; i<lenght ; ++i) {		// считывает данные
		SPDR=0xff;
		*(data+i) = SPDR;
		while (!(SPSR<<7));
		}

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;
	}




void rfm_22_write_burst (uint8_t address, uint8_t *data, uint8_t lenght) {

	register uint8_t i=0;

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address | (1 << 7);		 // записывает адрес

	while(!(SPSR<<7));

	for(i=0 ; i<lenght ; ++i) {		// записывает данные
		SPDR = *(data+i);
		while (!(SPSR<<7));
		}

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;
	}




void rfm_22_write(unsigned char address,unsigned char data) {

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address | (1<<7);		 // записываем адрес
	while (!(SPSR<<7));

	SPDR = data;         			// записывает данные
	while (!(SPSR<<7));

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;
	}




uint8_t rfm_22_read(unsigned char address) {

	unsigned char data;

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address;		 		// записывает адрес
	while (!(SPSR<<7));

	SPDR = 0xff;         		// записывает пустые данные

	while (!(SPSR<<7));

	data = SPDR;

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;

	return data;
	}



/*
void rfm_22_init_pn9 (void) {

	rfm_22_init();
	rfm_22_write(0x71, 0x1b);
	}
*/



uint8_t rfm_22_init(void) {

		uint8_t		init_table_io[3] = {0xd2, 0xdc, 0xd6},
					init_table_freq[6] = {0x5e, 0x01, 0x5d, 0x86, 0x03, 0x7e},
					init_table_ph[4] = {0x11, 0x46, 0x0a, 0x20},
					init_table_synch[4] = {0x28, 0x15, 0x23, 0x42},
					init_table_mod[3] = {0x0c, 0x23, 0x50},
					init_table_freqdiv[3] = {0x73, 0x64, 0x00};

	uint8_t error;

//	RFM_22_SDN_DDR |= RFM_22_SDN_EN;					//включает RFM
//	RFM_22_SDN_PORT &= ~RFM_22_SDN_EN;

	RFM_22_SPI_DDR |= RFM_22_SPI_PIN; 					//инициализация SPI
	RFM_22_SPI_EN_DDR |= RFM_22_SPI_EN_PIN;
	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;
	SPCR = RFM_22_SPCR;

	rfm_22_write(0x07,0x00); 							// режим standby

	rfm_22_write(0x05,0xff);							//включает прерывания
	rfm_22_write(0x06,0xff);

	rfm_22_read(0x03);						 			// обнуляет прерывания
	rfm_22_read(0x04);

	rfm_22_write(0x09,0x7f);							//емкость 12,5пФ

	rfm_22_write_burst(0x0b, init_table_io, 3);			//порты вода-вывода

	rfm_22_write(0x6d,0x1f);    						// мощность 13dBm

	rfm_22_write(0x1c,0x95);
	rfm_22_write(0x1d,0x40);

	rfm_22_write_burst(0x20, init_table_freq, 6);		//частоты

	rfm_22_write(0x27,0x1e);							//порог RSSI

	rfm_22_write(0x30, 0x8d);
	rfm_22_write_burst(0x32, init_table_ph, 4);			//обработчик пакетов

	rfm_22_write_burst(0x36, init_table_synch, 4);		//синхронизация

	rfm_22_write(0x3d,master_address);
	rfm_22_write(0x42, device_address);					//адрес устройства

//	rfm_22_write(0x43,0xff);							//маска адреса

	rfm_22_write(0x6e,0x10);							//скорость передачи
	rfm_22_write(0x6f,0x62);

	rfm_22_write_burst(0x70, init_table_mod, 3);		//модуляция

	rfm_22_write_burst(0x75, init_table_freqdiv, 3);	//конфигурация несущей частоты и девиации

	rfm_22_write(0x7e,0x00);							//порог заполнения fifo приемника

	error=rfm_22_read(0x20);

	if (  *init_table_freq != error ) error=1;			// проверка модуля
	else error=0;

	return error;
	}




void rfm_22_transmit(uint8_t startn, unsigned char lenght) {

	rfm_22_write(0x3e,lenght);							//размер пакета данных

	if (startn) {										//если указатель буфера пакета не 0
		rfm_22_write(0x08,0x01);						//стирает fifo
		rfm_22_write(0x08,0x00);

		rfm_22_write_burst(0x7f,buffer+startn,lenght);	//отправляет пакет в fifo
		_delay_ms(2);									//задержка чтобы приемник успел обработать предыдущий пакет
		}
	else if ( lenght==0 ) {	 							//если размер пакета 0
		rfm_22_write(0x08,0x01);						//стирает fifo
		rfm_22_write(0x08,0x00);

		rfm_22_write(0x3e,0x01);						//костыль из за долбанутых настроек порта RFM
		rfm_22_write(0x7f,0xee);
		}

//	rfm_22_write(0x07,0x04);							// вкл приемник
//	while(!(RFM_22_GPIO0_PIN & RFM_22_GPIO0_EN));		// проверяет эфир

	rfm_22_write(0x07,0x09);							//вкл режим передатчика

	}


/*

unsigned char rfm_22_receive(void) {

	rfm_22_read(0x03);        		  		//стирает прерывания
	rfm_22_write(0x08,0x02);				//стирает fifo
	rfm_22_write(0x08,0x00);

	rfm_22_write(0x07,0x05);				//режим приемника

	while ( !(rfm_22_read(0x03) & 0x02) );	//ожидает пакета

	lenght=rfm_22_read(0x4b);

	RFM_22_SPI_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = 0x7f;
	while (!(SPSR<<7));

	for (count=0 ; count<lenght ; count++ ) {
		SPDR = 0xff;
		while (!(SPSR<<7));
		buffer[count] = SPDR;
		}

	RFM_22_SPI_PORT |= RFM_22_SPI_EN_PIN;

	return lenght;
	}

*/


uint8_t rfm_22_receive(uint8_t startn) {

	uint8_t lenght;

	lenght = rfm_22_read(0x4b);							//считывает длину пакета

	rfm_22_read_burst(0x48, header , 3);				//считывает заголовок
	rfm_22_read_burst(0x7f, buffer+startn, lenght);		//считывает данные

	rfm_22_write(0x08,0x02);							//стирает fifo
	rfm_22_write(0x08,0x00);

	rfm_22_write(0x07,0x05);

	return lenght;
	}

