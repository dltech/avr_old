
#define MODBUS_PACK_LENGHT 64

#define REGUEST_WAIT_CYCLES	320



extern volatile uint8_t device_address;

extern void led_flash(register uint8_t i, uint8_t type);

uint8_t ping(uint8_t command, volatile uint8_t *ping_address, uint8_t ndevice);
