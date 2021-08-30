
#include <stdlib.h>
#include <avr/io.h>
#include "rfm_22_23.h"
#include <util/delay.h>

//#include <string.h>

#include "lcd.h"



unsigned char buffer[len], header[3], master_address=0xaa, device_address=0xaa;

int main (void) {

	unsigned char i;

	lcd_init(LCD_DISP_ON);

//	strcpy(buffer, "eeeeee");
	i = rfm_22_init();

	if (i) lcd_puts("init failed");
	else lcd_puts("init success");

	rfm_22_transmit(1,5);



	}
