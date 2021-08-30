
#define RS485_DEFAULT_BAUDRATE	9               // здесь задается скорость RS485
#define	RS485_UCSRA	0x02
#define	RS485_UCSRB	0x98
#define	RS485_UCSRC	0x8e

#define RS485_TCCR0 0x04                        // конфигурация таймера, по которому будет определяться время между символами

#define RS485_DDR	    DDRC                    // определяется порт, управляющий приемом/передачей
#define RS485_PORT	    PORTC
#define RS485_EN_PIN	0x20


#define SYMBOL_DELAY_TIME  54	//14
#define PACKET_DELAY_TIME  125	//31




extern volatile uint16_t npack;
extern volatile uint8_t buffer[256], packet_condition;



void rs485_init(void);

void rs485_set_baudrate(uint8_t nbaudrate);

void rs485_receive_packet(void);

void rs485_transmit_packet(void);
