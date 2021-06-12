#include "globals.h"
#include <util/crc16.h>

#include "rfm22.h"

/** библиотека для RFM23 и RFM22 
	@author Михаил Белкин 
*/


/* Записывает по SPI data по адресу address */
void rfm22_write (unsigned char address,unsigned char data)
{
	RFM22_PORT &= ~(1<<NSEL);

	SPDR = address | (1<<7);		 // записывает адрес
	while (!(SPSR<<7));

	SPDR = data;         			// записывает данные
	while (!(SPSR<<7));

	RFM22_PORT |= (1<<NSEL);
}


/* Считывает по SPI данные data по адресу address */
uint8_t rfm22_read(unsigned char address)
{
	unsigned char data;

	RFM22_PORT &= ~(1<<NSEL);

	SPDR = address;		 		// записывает адрес
	while (!(SPSR<<7));

	SPDR = 0xff;         		// записывает пустые данные
	while (!(SPSR<<7));
	data = SPDR;

	RFM22_PORT |= (1<<NSEL);
	return data;
}


/* Записывает последовательно lenght байт по SPI из *data по адресу address */
void rfm22_write_burst (uint8_t address, volatile uint8_t *data, uint8_t lenght)
{
	register uint8_t i=0;

	RFM22_PORT &= ~(1<<NSEL);

	SPDR = address | (1 << 7);		 // записывает адрес
	while(!(SPSR<<7));

	for(i=0 ; i<lenght ; ++i) {		// записывает данные
		SPDR = *(data+i);
		while (!(SPSR<<7));
		}

	RFM22_PORT |= (1<<NSEL);
}


/* Считывает последовательно lenght байт по SPI, начиная с адреса address, в *data */
void rfm22_read_burst  (uint8_t address, volatile uint8_t *data, uint8_t lenght)
{
	register uint8_t i=0;

	RFM22_PORT &= ~(1<<NSEL);

	SPDR = address;		 			// записывает адрес
	while (!(SPSR<<7));

	for(i=0 ; i<lenght ; ++i) {		// считывает данные
		SPDR=0xff;
		while (!(SPSR<<7));
		*(data+i) = SPDR;
		}

	RFM22_PORT |= (1<<NSEL);
}



uint8_t rfm22_init(void)
{
	RFM22_DDR |= RFM22_MASK;					//включает RFM
	RFM22_PORT |= (1<<NSEL) | (1<<NIRQ);
	RFM22_PORT &= ~(1<<SDN) ;

	SPCR = RFM22_SPCR;
	_delay_ms(100);

    rfm22_write(OPERATING_MODE1_P, STANDBY);

    rfm22_write(OSC_LOAD_CAP_P, 0x7f);
    rfm22_write(LOW_BATTERY_THR_P, LOW_BATTERY_THR);
    rfm22_write(IF_FILTER_BW_P, IF_FILTER_BW);

    rfm22_write(CLK_REC_OVER_P, CLK_REC_OVER);
    rfm22_write(CLK_REC_OFFSET2_P, CLK_REC_OFFSET2);
    rfm22_write(CLK_REC_OFFSET1_P, CLK_REC_OFFSET1);
    rfm22_write(CLK_REC_OFFSET0_P, CLK_REC_OFFSET0);
    rfm22_write(CLK_REC_CRGAIN1_P, CLK_REC_CRGAIN1);
    rfm22_write(CLK_REC_CRGAIN0_P, CLK_REC_CRGAIN0);

    rfm22_write(RSSI_THRESHOLD_P, RSSI_THRESHOLD);

    rfm22_write(AFC_LIMITER_P, AFC_LIMITER);
    rfm22_write(DATA_ACC_CTRL_P, DATA_ACC_CTRL);
    rfm22_write(HEADER_CTRL1_P, HEADER_CTRL1);
    rfm22_write(HEADER_CTRL2_P, HEADER_CTRL2);
    rfm22_write(PREAMBLE_LENGHT_P, PREAMBLE_LENGHT);
    rfm22_write(PREAMBLE_DETCTR_P, PREAMBLE_DETCTR);
    rfm22_write(SYNCH_WORD3_P, SYNCH_WORD3);
    rfm22_write(SYNCH_WORD2_P, SYNCH_WORD2);

    rfm22_write(INV_PREAMB_THR_P, INV_PREAMB_THR);
    rfm22_write(OSC_POR_CTRL_P, OSC_POR_CTRL);
    rfm22_write(TX_POWER_P, TX_POWER);
    rfm22_write(TX_DATA_RATE1_P, TX_DATA_RATE1);
    rfm22_write(TX_DATA_RATE0_P, TX_DATA_RATE0);
    rfm22_write(MODULAT_CTRL1_P, MODULAT_CTRL1);
    rfm22_write(MODULAT_CTRL2_P, MODULAT_CTRL2);
    rfm22_write(FREQ_DEVIATION_P, FREQ_DEVIATION);
    rfm22_write(FREQ_BAND_SEL_P, FREQ_BAND_SEL);
    rfm22_write(CARRIER_FREQ1_P, CARRIER_FREQ1);
    rfm22_write(CARRIER_FREQ2_P, CARRIER_FREQ2);
    rfm22_write(FREQ_HOP_CHSEL_P, FREQ_HOP_CHSEL);
    rfm22_write(FREQ_HOP_STSIZE_P, FREQ_HOP_STSIZE);

    if ( SYNCH_WORD3 != rfm22_read(SYNCH_WORD3_P) ) return 1;			// проверка модуля
	else                                            return 0;
}



void rfm22_transmit_start(uint8_t nbytes)
{
    uint8_t i;
    uint16_t crc=0xff;

    rfm22_allclean();

    for(i=0 ; i<nbytes ; ++i)   _crc16_update(crc, pack_buffer[i]);
    pack_buffer[nbytes++] = (uint8_t)(crc>>8);
    pack_buffer[nbytes++] = (uint8_t)crc;

    rfm22_write_burst(FIFO_ACCESS_P, pack_buffer, nbytes);
    rfm22_write(TX_PACK_LENGHT_P, nbytes);

    rfm22_write(OPERATING_MODE1_P, (1<<TX_ON));
}


uint8_t rfm22_transmit_complete(void)
{
    uint8_t error = 0;

    if ( ((1<<PACKET_SENT) & rfm22_wait_event(1<<PACKET_SENT)) == 0 )
        error = 2;

    rfm22_sleep();
    return error;
}



void rfm22_receive_start(void)
{
    rfm22_allclean();
    rfm22_write(OPERATING_MODE1_P, (1<<RX_ON));
}



uint8_t rfm22_receive_complete(void)
{
    uint8_t i, error = 0;
    uint16_t crc=0xff;

    if ( ((1<<PACKET_RECEIVED) & rfm22_wait_event(1<<PACKET_RECEIVED)) == 0 )
        return 2;                                                               // возвращает 2 если не дождался пакета

    pack_size = rfm22_read(RX_PACK_LENGHT_P);
    rfm22_read_burst(FIFO_ACCESS_P, pack_buffer, pack_size);

    for(i=0 ; i<(pack_size-2) ; ++i)   _crc16_update(crc, pack_buffer[i]);

    if( ( ((uint8_t)crc) != pack_buffer[--pack_size] )  ||  ( ((uint8_t)(crc>>8)) != pack_buffer[--pack_size] ) )
        error = 1;                                                               // возвращает 1 если ошибка контрольной суммы

    rfm22_sleep();
    return error;
}



uint8_t rfm22_wait_event (uint8_t event)
{
    uint8_t interrupt = 0;
    TCNT1 = 0;

    while (((event & interrupt) == 0) & (TCNT1<RFM22_WAIT_TIME))
        if ((RFM22_PIN & (1<<NIRQ)) == 0)
            interrupt = rfm22_read(INTERRUPT_ST1_P);

    return interrupt;
}


// очистка прерываний и всех буферов
void rfm22_allclean(void)
{
    rfm22_read(INTERRUPT_ST1_P);
    rfm22_read(INTERRUPT_ST2_P);
    rfm22_write(OPERATING_MODE2_P, (1<<TX_FIFO_CLR) | (1<<RX_FIFO_CLR) );
    rfm22_write(OPERATING_MODE2_P, 0x00);
}



void rfm22_sleep(void)
{
    rfm22_write(OPERATING_MODE1_P, STANDBY);
    rfm22_allclean();
}
