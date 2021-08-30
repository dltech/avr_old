#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include <string.h>
#include <avr/eeprom.h>


#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
#include "route.h"


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

#define INIT_MCUCR	0x03
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
uint8_t   address_table_eep[256]    EEPROM_SECTION;
uint8_t   device_address_eep	    EEPROM_SECTION  = 0x77;
uint8_t   master_address_eep	    EEPROM_SECTION  = 0;



volatile uint8_t buffer[256], header[3], device_type, nretx, nmodem, modem_address[48], retx_address[16], device_address, master_address, packet_condition;

volatile uint8_t npack;


void reset_slave (void);
void reset_master (void);

void led_flash(register uint8_t i,uint8_t tupe);

void init (void);



/* отладочная функция (тупо мигает светодиодами) */

void led_flash(register uint8_t i, uint8_t type) {
	uint8_t led = LED_PIN, flash1, flash2;
	switch(type) {
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

	init();									// инициализирует модем


	while(1) {

		rfm_22_write(0x07,0x05);            // переключает RFM в режим приемника

        rfm_22_write(0x08,0x03);										// стирает fifo
        rfm_22_write(0x08,0x00);

		GIFR = 0xc0; 						// обнуляет прерывания
		sei();								// включает прерывания

		DDRD &= ~0x0C;

		MCUCR |= SLEEP_ENABLE;				// уводит контроллер в спячку
		}
	}




/*   обработчики прерываний   */

ISR (USART_RXC_vect) {                              // если модем разбудил пакет, пришедший по RS485

	cli();                                          // выключает прерывания

	rfm_22_write(0x08,0x01);						//стирает fifo
	rfm_22_write(0x08,0x00);

	rs485_receive_packet();                         // принимает пакет по RS485

	if ( npack > 0 ) {								// если пакет принят
		switch (device_type) {
			case 1 : RS485_PORT |= RS485_EN_PIN;
					transmit_data_packet();			//отсылает пакет по радиоканалу
					break;
			case 3 : master_transmit();	break;		//отсылает пакет по радиоканалу
			}
		}
	}




ISR (INT0_vect) {									// если RFM принял пакет

	cli();                                          // выключает прерывания

	led_flash(2,2);

	switch (device_type) {
		case 1 : packet_handler_slave();	break;	//запускает обработчики пакетов
		case 2 : packet_handler_master();	break;
		case 3 : packet_handler_retx();  	break;
		}
	}




ISR (INT1_vect) {									//если была нажата кнопка reset

	cli();

//	BUTTON_PORT |= BUTTON_EN;						// вкл подтягивающий резистор на кнопке

	_delay_ms(10);									//задержка от дребезга

	if (~(BUTTON_PIN & BUTTON_EN)) {
		led_flash(2,3);
		switch (device_type) {
			case 1 : reset_slave();		break;		//в случае slave - сброс всего
			case 2 : reset_master();	break;		//в случае master - дополнение списка устройств
			case 3 : reset_master();	break;		//в случае ретранслятора - аналогично
			}
		}
	}






/*  функции сброса  */

void reset_slave (void) {

	eeprom_write_byte(&master_address_eep, 0);			// в случае ведомого устройства сбрасывает master_address и списки ведущего
	eeprom_write_byte(&nmodem_eep, 0);
	eeprom_write_byte(&nretx_eep, 0);
	master_address=0;
	nmodem=0;
	nretx=0;
	LED_PORT |= RED_LED; 								// включает красный светодиод
	}




void reset_master (void) {

	register uint8_t i=0;
	uint8_t temp;

	temp = nmodem;
	nmodem = ping(0x10, modem_address, nmodem);												    // опрашивает устройства, дополняет список
	for (i=temp ; i<nmodem ; ++i)	eeprom_write_byte(&modem_address_eep[i],modem_address[i]);	// записывает в eeprom
	if (nmodem + nretx)  	LED_PORT &= ~RED_LED;												// если количество устройств в сети не 0 выключает красный светодиод
	else	LED_PORT |= RED_LED;																// иначе включает
	led_flash(2,1);
	}




/*  инициализация модема  */

void init (void) {

	_delay_ms(500);

	register uint8_t i;

	uint8_t temp;

	LED_DDR |= GREEN_LED + RED_LED;				// инициалиация светодиодов
	JUMPER_PORT |= JUMPER_RETX + JUMPER_MS;		//вкл подтягивающие резисторы на перемычках

	_delay_ms(1);                               // хрен знает зачем

	if(JUMPER_PIN & JUMPER_RETX) {				// считывает состояния перемычек

		if(JUMPER_PIN & JUMPER_MS) {
			device_type = 1;										//тип устройства - модем конечного устройства
			master_address = eeprom_read_byte(&master_address_eep);	//считывает master_address
			if (master_address == 0) LED_PORT |= RED_LED;			//если устройство не привязано к сети - включает красный светодиод
			}
		else {
			device_type = 2;										//тип устройства - модем на сервере
            master_address = 0xff;

            temp = read_device_list();

			led_flash(2,1);
			if (temp == 0) LED_PORT |= RED_LED;			            // если нет устройств привязанных к сети - включает красный светодиод
			}

		}

	else {
		if(JUMPER_PIN & JUMPER_MS) {
			device_type=3;											//тип устройства - ретранслятор

			if (master_address == 0) LED_PORT |= RED_LED;			//если устройство не привязано к сети - включает красный светодиод
			}
		else {
			device_type=4;                                          // тип устройства - четвертый (отладочный режим)
			}
		}

	JUMPER_PORT &= ~(JUMPER_MS + JUMPER_RETX);	// выкл подтягивающие резисторы на перемычках

	device_address = eeprom_read_byte(&device_address_eep);	// считывет адрес устроства


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
			device_type = 0;

			led_flash(2,3);
			}
		}


	if ( (device_address == 0) | (device_type == 0) ) {

		LED_PORT |= RED_LED;			// если адрес устройства отсутствует, или RFM не была инициализирована включает красный светодиод
		device_type = 0;
		}
	else {

		LED_PORT |= GREEN_LED; 			// если RFM инициализирован успешно и адрес устройтва успешно считан - включает зеленый светодиод
//		BUTTON_PORT |= BUTTON_EN;		// вкл подтягивающий резистор на кнопке, не включаем, тк с внутренней подтяжкой работа нестабильна
		_delay_us(100);					//хрен его зает зачем

		TCCR1B = INIT_TCCR1B;           //инициализация таймера для формирования задержек

		MCUCR = INIT_MCUCR;				// конфигурирует внешние прерывания
		GICR = INIT_GICR;

		rfm_22_write(0x07,0x05);		//вкл приемник
		_delay_us(200);

		}
	}
