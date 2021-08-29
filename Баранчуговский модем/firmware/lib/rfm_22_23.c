#include <avr/io.h>
#include <util/delay.h>
#include <rfm_22_23.h>
#include <lcd.h>
#include <stdlib.h>

//uint_8t init_table_freq[



void rfm_22_write_burst (unsigned char address, unsigned char data[]) {
	
	SPCR=RFM_22_SPCR;
	
	RFM_22_SPI_PORT &= ~RFM_22_SPI_PIN;
	
	SPDR = address | (1 << 7);		 // записывает адрес fifo
	
	while(!(SPSR<<7));
	
	while (*data!=0) {
		SPDR = *data;
		++data;
		while (!(SPSR<<7));
		}
	
	RFM_22_SPI_PORT |= RFM_22_SPI_PIN;
	SPCR=0;
	}




void rfm_22_write(unsigned char address,unsigned char data) {
	
	SPCR=RFM_22_SPCR;
	
	RFM_22_SPI_PORT &= ~RFM_22_SPI_PIN;
	
	SPDR = address | (1<<7);		 // записываем адрес
	while (!(SPSR<<7)); 
	
	SPDR = data;         // записываем данные
	while (!(SPSR<<7));	
	
	RFM_22_SPI_PORT |= RFM_22_SPI_PIN;
	
	SPCR=0;	
	_delay_ms(5); 
	}
	
	
	
	
unsigned char rfm_22_read(unsigned char address) { 
	
	unsigned char data;
	
	SPCR=RFM_22_SPCR;
	
	RFM_22_SPI_PORT &= ~RFM_22_SPI_PIN;
	
	SPDR = address;		 // записываем адрес
	while (!(SPSR<<7));
	
	SPDR = 0xff;         // записываем пустые данные
	while (!(SPSR<<7));	

	data = SPDR;
	
	RFM_22_SPI_PORT |= RFM_22_SPI_PIN;	
	
	SPCR=0;
	_delay_ms(5); 
	
	return data;
	}
	
	
	
	
/*	
void rfm_22_init_pn9 (void) {
	
	rfm_22_init();
	rfm_22_write(0x71, 0x1b);
	}
*/
	
	
	
void rfm_22_init(void) {
	
	RFM_22_SPI_PORT|= RFM_22_SPI_PIN;

	_delay_ms(20);
	
	rfm_22_write(0x07,0x00); 	// режим standby

	rfm_22_write(0x05,0xff);	//включаем прерывания
	rfm_22_write(0x06,0xff);

	rfm_22_read(0x03); 			// обнуляем прерывания
	rfm_22_read(0x04); 	
	
	rfm_22_write(0x09,0x7f);	//емкость 12,5пФ
	
	rfm_22_write(0x6d,0x1f);    // мощность 13dBm
	
	rfm_22_write(0x1c,0x06);
	rfm_22_write(0x1d,0x40);

	rfm_22_write(0x20,0xfa);	//частоты
	rfm_22_write(0x21,0x00);
	rfm_22_write(0x22,0x83);
	rfm_22_write(0x23,0x12);
	rfm_22_write(0x24,0x00);	
	rfm_22_write(0x25,0x56);
//	rfm_22_write(0x2a,0x28);
	
	rfm_22_write(0x30,0xac);	//режим fifo, crc вкл 
	rfm_22_write(0x32,0x00);	//адрес выключен
	rfm_22_write(0x33,0x40);	//адрес отключен, длина пакета вкл, синхронизация 1 байт
	rfm_22_write(0x34,0x0a);	//длина заголовка
	rfm_22_write(0x35,0x10);	//длина определяемого заголовка
	
	rfm_22_write(0x36,0x2d);	//синхронизационное поле 3
	rfm_22_write(0x37,0xd4);	//синхронизационное поле 2
	rfm_22_write(0x39,0x00);
	
	rfm_22_write(0x3a,0x09);
	rfm_22_write(0x3b,0x08);
	rfm_22_write(0x3c,0x07);
	rfm_22_write(0x3d,0x06);
	
	rfm_22_write(0x3f,0x00);
	rfm_22_write(0x40,0x00);
	rfm_22_write(0x41,0x00);
	rfm_22_write(0x42,0x00);
	
	rfm_22_write(0x43,0xff);
	rfm_22_write(0x44,0xff);
	rfm_22_write(0x45,0xff);
	rfm_22_write(0x46,0xff);

	rfm_22_write(0x6e,0x83);
	rfm_22_write(0x6f,0x12);
	
	rfm_22_write(0x70,0x2c);
	rfm_22_write(0x71,0x23);
	rfm_22_write(0x72,0x50);
	
	rfm_22_write(0x75,0x53);
	rfm_22_write(0x76,0x4b);
	rfm_22_write(0x77,0x00);





	_delay_ms(10);
	
	}
	
	
	
	
void rfm_22_transmit(unsigned char lenght) {
	
	rfm_22_write(0x08,0x01);			//стирает fifo
	rfm_22_write(0x08,0x00);
	
	rfm_22_write(0x3e,lenght);			//количество данных
	
	rfm_22_write_burst(0x7f,output_packet);	//отправляем пакет в fifo
	
	rfm_22_write(0x07,0x04);					// вкл приемник
	while(!(PINC & 0x20));						// проверяет эфир
	
	rfm_22_write(0x07,0x09);			//вкл режим передатчика
	
	_delay_ms(200);
	
	}
 	
	
	
	
unsigned char rfm_22_receive(void) {
	
	char disp[8];
	unsigned char a,lenght, count;
	
	
	rfm_22_read(0x03);          //стирает прерывания
	rfm_22_write(0x08,0x02);	//стирает fifo
	rfm_22_write(0x08,0x00);

	rfm_22_write(0x07,0x05);	//режим приемника
	
	_delay_ms(20);
	
	while ( !(rfm_22_read(0x03) & 0x02) );		//ожидает пакета
	
	lenght=rfm_22_read(0x4b);
	
	SPCR=RFM_22_SPCR;
	
	RFM_22_SPI_PORT &= ~RFM_22_SPI_PIN;
	
	SPDR = 0x7f;
	while (!(SPSR<<7));
	
	for (count=0 ; count<lenght ; count++ ) { 	
		SPDR = 0xff;
		while (!(SPSR<<7));
		input_packet[count] = SPDR;
		}
	
	RFM_22_SPI_PORT |= RFM_22_SPI_PIN;
	
	SPCR=0;

	
	return lenght;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	