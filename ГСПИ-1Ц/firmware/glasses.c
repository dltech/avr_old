/**	Светодиодная мигалка для экспериментов над людьми, ГСПИ-1Ц.
	Состоит из одного исходника.	
	@author Михаил Белкин  **/

#define F_CPU 4000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <lcd.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#define freqnmin	100
#define freqnmax	2500

#define pushtime	9000
#define pushdelay	4000

// here are settings
#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))
uint16_t  dummy         EEPROM_SECTION  = 0;       // avoid using lowest addresses
uint8_t   colour_eep    EEPROM_SECTION  = 0;		// номер цвета
uint8_t   brightn_eep   EEPROM_SECTION  = 50;		// яркость в процентах
uint8_t   bright_eep    EEPROM_SECTION  = 0x7f;		// яркость в отсчетах шим
uint16_t  freqn_eep     EEPROM_SECTION  = freqnmin;	// частота в герцах

uint16_t  freq, freqn;
uint8_t ff[4], led, run[3]={0x13, 0x0B, 0x07}, runn, menun, brightn, colour, chr, scount, init;

void chargeCheck(void);

void delay_timer(unsigned int time) {
	TCNT1=0;
	while(TCNT1<time);
}


// printf imitation
void print_number (unsigned int num, uint8_t n) {
	chr=0;
	
	if (num==0) lcd_puts("  0");
	while (num>0) {
		ff[chr] = num % 10 + '0';
		num /= 10;
		++chr;
	}
    --chr;
	runn=n-chr+1;

	while((--runn)>0) lcd_putc(' ');

	for( ; chr<254 ; --chr) {
		lcd_putc(ff[chr]);
		if((chr==2) & (n==3)) lcd_putc('.');
    }
}

// проверяет состояние аккумулятора по суервайзеру напряжения
// если зарядился, то выводит надпись зарядка окончена
// если зарядное не подключено, то ничего не делает
void chargeCheck(void)
{
	// идет зарядка
	if ( PIND & (1<<6) ) {
		lcd_home();
		lcd_puts("  �㵿");
		lcd_gotoxy(0,1);
		lcd_puts(" �ap��");
	}
	// зарядка окончена
	while( (PIND & (1<<6)) && (PIND & 1) ) {
		if ( PINB & (1<<7) ) {
			lcd_home();
			lcd_puts(" �ap��");
			lcd_gotoxy(0,1);
			lcd_puts("o�o��e�");
		}
	}
	lcd_clrscr();
}

// функция вывода меню на экран
void menu(void) {
	PORTD=0X03;
	TCNT1=0;
	scount=0;
	lcd_gotoxy(7,0);
	switch (menun) {
		case 0 : lcd_putc(171); break;
		case 1 : lcd_putc(225); break;
		case 2 : lcd_putc(177); break;
	}

	while(TCNT1<65535) {

		if ( ((PIND&1)==0) || (init==0) ) {
			if (init==1)
				delay_timer(pushtime);
			++menun;
			++scount;
			lcd_gotoxy(7,0);
			switch (menun) {
				case 0 : lcd_putc(171); break;
				case 1 : lcd_putc(225); break;
				case 2 : lcd_putc(177); break;
				default: menun=0;
						 lcd_putc(171); break;
				}
			if ( ((PIND&1)==0) && (init==1) )
				delay_timer(pushdelay);
			TCNT1=0;
			}


		if ( ((PIND&1<<1)==0) || (init==0) ) {

			if (init==1) {
				delay_timer(pushtime);
				scount=0;
				}

			switch (menun) {
				case 0 :	if (init==1)
								freqn+=5;

							if (freqn>freqnmax)
								freqn=freqnmin;
							lcd_home();
							print_number(freqn,3);
							lcd_putc(161);
							lcd_putc(229);
							break;

				case 1 : 	if (init==1)
								++colour;
							chr=0;

							lcd_gotoxy(0,1);
							switch(colour) {
								case 0 : led=0x03;
										 lcd_puts("�ce");   break;
								case 1 : led=0x0F;
										 lcd_puts("�p ");   break;
								case 2 : led=0x17;
										 lcd_puts("�e�");   break;
								case 3 : led=0x1B;
										 lcd_puts("c��");   break;
								case 4 : lcd_puts("�e�");   break;
								default : colour=0;
										  led=0x03;
										  lcd_puts("�ce");  break;
								}
							break;

				case 2 : 	if (init==1) {
							brightn+=5;
							OCR0B+=0x0C;
							}

							lcd_gotoxy(4,1);
							if (brightn>100) {
								brightn=0;
								OCR0B=0x00;
								}
							print_number(brightn,2);
							lcd_putc('%');
							break;
				}

			if ( ((PIND&1<<1)==0) && (init==1) )
			delay_timer(pushdelay);
			TCNT1=0;
			}

		if ((scount==3) && (init==0))
			break;
	}


	freq=3125000/freqn;


	if (scount>18) {
		lcd_gotoxy(7,0);
		lcd_putc(67);
		eeprom_write_byte(&colour_eep, colour);
		eeprom_write_byte(&brightn_eep, brightn);
		eeprom_write_byte(&bright_eep, OCR0B);
		eeprom_write_word(&freqn_eep, freqn);
		_delay_ms(2000);
	}

	lcd_gotoxy(7,0);
	lcd_putc(32);
}


int main(void) {

    DDRA=0x00;
	DDRB=0x00;
	DDRD=0x3C;
	PORTA=0x04;
	PORTB=0x00;
	PORTD=0x03;
	ACSR=0x80;

	TCNT1=0;
	TCCR1A=0x00;
	TCCR1B=0x03;

	lcd_init(LCD_DISP_ON);
	chargeCheck();

	TCNT0=0;
	TCCR0A=0x23;
	TCCR0B=0x01;
	OCR0B=eeprom_read_byte(&bright_eep);
	chr=0;
	brightn=eeprom_read_byte(&brightn_eep);
	menun=0;
	runn=0;
	colour=eeprom_read_byte(&colour_eep);
	freqn=eeprom_read_word(&freqn_eep);

	init=0;
	menu();
	init=1;

	while(1) {
		if (TCNT1>=freq) {
			TCNT1=0;
			// собственно само мигание
			if (colour==4) {
				++runn;
				if (runn==3) runn=0;
				PORTD=run[runn];
			} else  PORTD=led^~PIND;
			// символ состояния (батарейка там заряена типо или нет)
			lcd_gotoxy(7,0);
			if (PIND&1<<6) {
				if (PINB&1<<7) {
					if (chr!=0x21) {
						lcd_putc(0x21);
						chr=0x21;
					}
				} else {
					if(chr!=126) {
						lcd_putc(126);
						chr=126;
					}
				}
			} else {
				if(chr!=32) {
					lcd_putc(32);
					chr=32;
				}
			}
		}

		// опрос кнопок и обновление меню
		if ((PIND&1)==0) {
			delay_timer(pushtime);
			menu();
		}

	}
}
