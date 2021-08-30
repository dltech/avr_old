#define MODEM_RECEIVE_COMMAND_TIME 1
#define MODEM_PACK_TRANSMIT_DELAY	81

#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))

extern volatile uint16_t npack;
extern volatile uint8_t buffer[256], header[3], connect_address, device_address, packet_condition;


/* обработчики пакетов */
void packet_handler(void);
