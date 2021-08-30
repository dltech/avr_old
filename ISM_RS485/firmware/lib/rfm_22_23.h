#include <stdint.h>

#define len 256
#define RFM_22_SPCR 0b01010011

#define RFM_22_SPI_PORT PORTB
#define RFM_22_SPI_PIN  0x04


extern unsigned char input_packet[len], output_packet[len];

void rfm_22_write(unsigned char address, unsigned char data);

unsigned char rfm_22_read(unsigned char address);

void rfm_22_write_burst(unsigned char address, unsigned char data[]);

void rfm_22_init(void);

void rfm_22_transmit(unsigned char lenght);

unsigned char rfm_22_receive(void);