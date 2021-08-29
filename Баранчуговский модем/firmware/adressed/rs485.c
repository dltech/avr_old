#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
//#include "route.h"




/* Установка скорости RS485 */

void rs485_set_baudrate(uint8_t nbaudrate) {

    uint8_t baudrate_table[13] = {207, 103, 51, 34, 25, 16, 12, 8, 6, 3, 1, 1, 0};

    UBRRL = baudrate_table[nbaudrate];
    }




void rs485_init(void) {

	UCSRA = RS485_UCSRA;				// конфигурация UART
	UCSRB = RS485_UCSRB;
	UCSRC = RS485_UCSRC;


    UBRRL = 103;
//	rs485_set_baudrate(RS485_DEFAULT_BAUDRATE);
	UBRRH = 0;

	TCCR0 = RS485_TCCR0;

	RS485_DDR |= RS485_EN_PIN;
//	RS485_PORT |= RS485_EN_PIN;			// режим передатчика
	}



void rs485_receive_packet(void) {

    packet_condition = 0xff;
	npack=0;

	TCNT0 = 0;

    connect_address = UDR;

	while(TCNT0<PACKET_DELAY_TIME) {
		if (UCSRA & 0x80) {

			if(TCNT0 > SYMBOL_DELAY_TIME)   packet_condition = 0;	// если задержка более 1,5 символов - пакет игнорируется

			*(buffer+npack)=UDR;								    // записывает байт в буфер
			if (npack<64) rfm_22_write(0x7f,*(buffer+npack));	    // записывает приятый байт в RFM
			++npack;

			TCNT0=0;											    // обнуляет таймер - счетчик
			}
		}

    if ((npack > 256) | (npack == 0)) packet_condition = 0;

    return;
	}





void rs485_transmit_packet(void) {

	register uint16_t i=0;

	RS485_PORT |= RS485_EN_PIN;                                     // переключает интерфейсную микросхему в режим передачи

	for (i=0 ; i<npack ; ++i) {

        while( !(UCSRA & 0x20) );
        UDR = *(i+buffer);
	    }

    UCSRA |= 0x40;
    while( !(UCSRA & 0x40) );                                       // ожидание передачи последнего байта

    RS485_PORT &= ~RS485_EN_PIN;                                    // переключает интерфейсную микросхему в режим приема

    i = UDR;                                                        // костыль из за отсутствия подтягивающего резистора на UART
    i = UDR;
    i = UDR;

    packet_condition = 0;                                           // после отправки пакета байт состояния обнуляется

	return;
	}

