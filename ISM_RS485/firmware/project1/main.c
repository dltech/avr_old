#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include <string.h>
#include <avr/eeprom.h>
#include <util/atomic.h>


#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
//#include "route.h"


#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))

/* перемычки */

#define JUMPER_DDR	DDRB
#define JUMPER_PORT PORTB
#define JUMPER_PIN	PINB
#define JUMPER_RETX 0x02
#define JUMPER_MS 	0x04

/* светодиоды */

#define LED_PIN		PIND
#define LED_DDR		DDRD
#define LED_PORT	PORTD
#define RED_LED 	0x20
#define GREEN_LED	0x40

/* регистры контроллера */

#define INIT_MCUCR	0x83
#define INIT_GICR	0xc0

#define SLEEP_ENABLE 0x80

#define INIT_TCCR1B	 0x05

/* кнопка */

#define BUTTON_DDR	DDRD
#define BUTTON_PORT	PORTD
#define BUTTON_PIN	PIND
#define BUTTON_EN 	0x08

/* инициализация eeprom */

uint16_t  dummy         		    EEPROM_SECTION  = 0;       // avoid using lowest addresses
uint8_t   device_address_eep	    EEPROM_SECTION  = 0x77;
uint8_t   connect_address_eep	    EEPROM_SECTION  = 0x77;


volatile uint16_t npack;
volatile uint8_t buffer[256], header[3], device_address, connect_address, packet_condition;



void led_flash(register uint8_t i,uint8_t tupe);

void init (void);



/* отладочная функция (тупо мигает светодиодами) */

void led_flash(register uint8_t i, uint8_t type) {
	uint8_t led = LED_PIN, flash1, flash2;
	switch (type) {
		case 0 : flash1=0;
				 flash2=0;
				 break;
		case 1 : flash1=RED_LED;
				 flash2=GREEN_LED;
				 break;
		case 3 : flash1=RED_LED;
				 flash2=0;
				 break;
		case 2 : flash1=GREEN_LED;
				 flash2=0;
				 break;
		}
	++i;
	while(--i) {
		LED_PORT |= flash1;
		LED_PORT &= ~flash2;
		_delay_ms(200);
		LED_PORT |= flash2;
		LED_PORT &= ~flash1;
		_delay_ms(200);
		}
	LED_PORT = led;
	_delay_ms(400);
	}




/* Основная функция, инициализирует модем, переключает RFM в режим приемника, включает прерывания, и уводит МК в спящий режим.
    Пакет приходит, и будит МК прерыванием, когда пакет будет обработан, проц опять заснет до следущего пакета. */

int main (void) {

    uint8_t eee;

	init();									// инициализирует модем


	while(1) {

        rfm_22_clear();

		GIFR = 0xc0; 						// обнуляет прерывания
        eee = UDR;

		sei();								// включает прерывания

		sleep_cpu();		        		// уводит контроллер в спячку
		}
	}




/*   обработчики прерываний   */

ISR (USART_RXC_vect) {                              // если модем разбудил пакет, пришедший по RS485

	cli();                                          // выключает прерывания

	rs485_receive_packet();                         // принимает пакет по RS485

    if (packet_condition == 0xff) {
        if (npack == 256) {
            rfm_22_write(0x3b,0x02);
            rfm_22_transmit_packet_256();
            }
        else {
            rfm_22_write(0x3b,0x01);
            rfm_22_transmit_packet();
            }
    packet_condition = 0x10;        // обнуляет состояние пакета для исключения еще одной пересылки
    }

    register uint8_t rubbish = UDR;

	return;
	}




ISR (INT0_vect) {									// если RFM принял пакет

	cli();                                          // выключает прерывания

    packet_handler();                               // запуск обработчика пакета

    GIFR = 0xc0; 						            // обнуляет прерывания

    return;
	}




ISR (INT1_vect) {									// если была нажата кнопка reset

	cli();

    led_flash(2,1);

    GIFR = 0xc0; 						            // обнуляет прерывания

    return;
	}




/*  инициализация модема  */

void init (void) {

	register uint8_t i;

	uint8_t temp;

    _delay_ms(50);

	LED_DDR |= GREEN_LED + RED_LED;				// инициалиация светодиодов

	device_address = eeprom_read_byte(&device_address_eep);	    // считывет адрес устроства

    connect_address = eeprom_read_byte(&connect_address_eep);   // считывает адрес устройства на радиоканале
    if (connect_address == 0)   LED_PORT |= RED_LED;


	temp = rfm_22_init();						// инициализация RFM

	if(temp == 1) {

		for (i=0 ; i<5 ; ++i) {
		RFM_22_SDN_PORT |= RFM_22_SDN_EN;		// если RFM не отвечает, выключает, пробует инициализировать еще 5 раз
		_delay_ms(2);
		temp = rfm_22_init();
		if (temp == 0) break;
		}

		if (temp == 1) {						// если все равно не отвечает - выключает RFM
			RFM_22_SDN_PORT |= RFM_22_SDN_EN;
			device_address = 0;

			led_flash(2,3);
			}
		}


	if ( device_address == 0 ) {
		LED_PORT |= RED_LED;			// если адрес устройства отсутствует, или RFM не была инициализирована включает красный светодиод
		}
	else {

		LED_PORT |= GREEN_LED; 			// если RFM инициализирован успешно и адрес устройтва успешно считан - включает зеленый светодиод
//		BUTTON_PORT |= BUTTON_EN;		// вкл подтягивающий резистор на кнопке, не включаем, тк с внутренней подтяжкой работа нестабильна

		TCCR1B = INIT_TCCR1B;           // инициализация таймера для формирования задержек

		MCUCR = INIT_MCUCR;				// конфигурирует внешние прерывания
		GICR = INIT_GICR;

        rs485_init();

		rfm_22_write(0x07,0x05);		// вкл приемник
		_delay_us(200);

        DDRD &= ~0x0c;

		}
	}
