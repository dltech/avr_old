#include <stdint.h>

#define len 256

#define RFM_22_GPIO0_PORT 	PORTC
#define RFM_22_GPIO0_DDR	DDRC
#define RFM_22_GPIO0_PIN	PINC
#define RFM_22_GPIO0_EN		0x08

#define RFM_22_GPIO1_PORT	PORTC
#define RFM_22_GPIO1_DDR	DDRC
#define RFM_22_GPIO1_PIN	PINC
#define RFM_22_GPIO1_EN		0x10

#define RFM_22_GPIO2_PORT	PORTD
#define RFM_22_GPIO2_DDR	DDRD
#define RFM_22_GPIO2_PIN	PIND
#define RFM_22_GPIO2_EN		0x08

#define RFM_22_SDN_PORT		PORTC
#define RFM_22_SDN_DDR		DDRC
#define RFM_22_SDN_PIN		PINC
#define RFM_22_SDN_EN		0x04

#define RFM_22_NIRQ_PORT 	PORTC
#define RFM_22_NIRQ_DDR	 	DDRC
#define RFM_22_NIRQ_PIN	 	PINC
#define RFM_22_NIRQ_EN		0x02	

#define RFM_22_SPI_PORT 	PORTB
#define RFM_22_SPCR 		0x50
#define RFM_22_SPI_DDR 		DDRB
#define RFM_22_SPI_PIN		0x2c

#define RFM_22_SPI_EN_DDR	DDRB
#define RFM_22_SPI_EN_PORT	PORTB
#define RFM_22_SPI_EN_PIN 	0x04


#define LED_DDR		DDRD
#define LED_PORT	PORTD
#define RED_LED 	0x20
#define GREEN_LED	0x40


volatile extern uint8_t buffer[len], header[3], master_address, device_address;

/*  SPI functions  */

void rfm_22_write(unsigned char address,unsigned char data);

uint8_t rfm_22_read(unsigned char address);

void  rfm_22_read_burst(uint8_t address, uint8_t *data, uint8_t lenght);

void rfm_22_write_burst(uint8_t address, uint8_t *data, uint8_t lenght);



uint8_t rfm_22_init(void);


/*  RX/TX functions  */

void rfm_22_transmit_packet(uint8_t startn, unsigned char lenght);

uint8_t rfm_22_read_packet(uint8_t startn);

