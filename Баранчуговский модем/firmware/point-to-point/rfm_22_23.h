
/* Настройки портов ввода вывода контроллера соединенных с портами RFM */

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
#define RFM_22_GPIO2_EN		0x04

/* Настройки лапы включения/выключения RFM */

#define RFM_22_SDN_PORT		PORTC
#define RFM_22_SDN_DDR		DDRC
#define RFM_22_SDN_PIN		PINC
#define RFM_22_SDN_EN		0x04

/* Настройки порта, соединенного с прерывнием RFM */

#define RFM_22_NIRQ_PORT 	PORTC
#define RFM_22_NIRQ_DDR	 	DDRC
#define RFM_22_NIRQ_PIN	 	PINC
#define RFM_22_NIRQ_EN		0x02

/* Настройки SPI */

#define RFM_22_SPCR 		0x51
#define RFM_22_SPI_PORT 	PORTB
#define RFM_22_SPI_DDR 		DDRB
#define RFM_22_SPI_PIN		0x2c

#define RFM_22_SPI_EN_DDR	DDRC
#define RFM_22_SPI_EN_PORT	PORTC
#define RFM_22_SPI_EN_PIN 	0x01

/* Время ожидания события */

#define RFM_22_WAIT_TIME    1024


volatile extern uint16_t npack;
volatile extern uint8_t buffer[256], header[3], device_address, connect_address, packet_condition;



/*  функции SPI  */

void rfm_22_write(unsigned char address,unsigned char data);

uint8_t rfm_22_read(unsigned char address);

void  rfm_22_read_burst(uint8_t address, volatile uint8_t *data, uint8_t lenght);

void rfm_22_write_burst(uint8_t address, volatile uint8_t *data, uint8_t lenght);




/* Функции связанные с управлением RFM */

uint8_t rfm_22_init(void);

void rfm_22_clear(void);

uint8_t rfm_22_wait_event(uint8_t event);




/*  функции приема и передачи  */

void rfm_22_transmit_command(uint8_t command);

void rfm_22_transmit_packet_256(void);

void rfm_22_transmit_packet(void);

void rfm_22_transmit_small_packet(uint8_t startn, unsigned char lenght);


void rfm_22_receive_packet_256(void);

void rfm_22_receive_packet(void);

uint8_t rfm_22_read_small_packet(uint8_t startn);

