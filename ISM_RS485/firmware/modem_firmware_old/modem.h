#define MODEM_RECEIVE_COMMAND_TIME 1
#define MODEM_PACK_TRANSMIT_DELAY	81

#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))


extern uint8_t   master_address_eep	EEPROM_SECTION;
extern volatile uint8_t buffer[256], header[3], device_type, nretx, nmodem, modem_address[48], retx_address[16], device_address, master_address, packet_condition, npack;


/* задержка*/
void delay_timer(uint8_t cycles);


/* обработчики пакетов */
void packet_handler_slave(void);

void packet_handler_master(void);

void packet_handler_retx (void);


/* функции приема пакета */
void receive_single_data_packet (void);

void receive_multiple_data_packet (void);


/* функции передачи пакета */
void transmit_data_packet(void);

void master_transmit(void);
