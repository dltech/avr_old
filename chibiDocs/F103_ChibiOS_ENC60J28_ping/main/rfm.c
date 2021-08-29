#include "hal.h"
#include "rfm.h"

#define rfm_spi_start(enc) spiSelect((rfmconf)->driver);
#define rfm_spi_stop(enc) spiUnselect((rfmconf)->driver);

/* Глобальные переменные необходимые для работы радиомодуля */
uint8_t packet_buffer[RFM69_BUFFER_SIZE - 2];		///< буфер, в котором хранится пакет
uint8_t rfm69_condition;							///< в ней хранится состояние радиомодуля в данный момент
uint8_t packet_size;								///< размер пакета

extern uint8_t internal_packet_buffer[64], internal_pack_size;

static void rfm_reset(void)
{
	// Reset RFM-chip. Time slot 100us and 5ms from RFM datasheet.
	chSysLock();
	palSetPad(RFM_RESET_PORT, RFM_RESET_PIN);
	chThdSleepMicroseconds(100);
	palClearPad(RFM_RESET_PORT, RFM_RESET_PIN);
	chSysUnlock();

	chThdSleepMilliseconds(5);
}

static void rfm_write(struct rfmconf *rfmconf, uint8_t addr, uint8_t data)
{
	uint8_t addres = (addr | 0x80);
	
	rfm_spi_start(rfmconf);
	spiSend(rfmconf->driver, 1, &addres);
	spiSend(rfmconf->driver, 1, &data);
	rfm_spi_stop(rfmconf);
}

static uint8_t rfm_read(struct rfmconf *rfmconf, uint8_t addr)
{
	uint8_t ret;
	
	rfm_spi_start(rfmconf);
	spiSend(rfmconf->driver, 1, &addr);
	spiReceive(rfmconf->driver, 1, &ret);
	rfm_spi_stop(rfmconf);
	
	return ret;
}

//static void rfm_write_direct(struct rfmconf *rfmconf, uint8_t *addr, uint8_t *data)
//{
//	uint8_t addres = (*addr | 0x80);
//	
//	rfm_spi_start(rfmconf);
//	spiSend(rfmconf->driver, 1, &addres);
//	spiSend(rfmconf->driver, 1, data);
//	rfm_spi_stop(rfmconf);
//}

//static void rfm_read_direct(struct rfmconf *rfmconf, uint8_t *addr, uint8_t *ret)
//{
//	rfm_spi_start(rfmconf);
//	spiSend(rfmconf->driver, 1, addr);
//	spiReceive(rfmconf->driver, 1, ret);
//	rfm_spi_stop(rfmconf);
//}

//static void rfm_write_burst(struct rfmconf *rfmconf, uint8_t *addr, uint8_t *data, uint8_t len)
//{
//	uint8_t addres = (*addr | 0x80);
//	
//	rfm_spi_start(rfmconf);
//	spiSend(rfmconf->driver, 1, &addres);
//	spiSend(rfmconf->driver, len, data);
//	rfm_spi_stop(rfmconf);
//}

//static void rfm_read_burst(struct rfmconf *rfmconf, uint8_t *addr, uint8_t *ret, uint8_t len)
//{
//	rfm_spi_start(rfmconf);
//	spiSend(rfmconf->driver, 1, addr);
//	spiReceive(rfmconf->driver, len, ret);
//	rfm_spi_stop(rfmconf);
//}

//uint8_t rfm_read_REG(struct rfmconf *rfmconf, uint8_t addr)
//{
//	uint8_t result;
//	rfm_read(rfmconf, addr);
//	return result;
//}

//void rfm_write_REG(struct rfmconf *rfmconf, uint8_t addr, uint8_t data)
//{
//	rfm_write(rfmconf, addr, data);
//}

uint8_t rfm_init(struct rfmconf *rfmconf)
{
	int j;

  //rfm69_mcu_init();
	
	rfm_reset();

//	RFM69 initialization
	rfm_write(rfmconf, REGOPMODE, REGOPMODE_DEF | STBY_MODE);
	rfm_write(rfmconf, REGDATAMODUL, REGDATAMODUL_DEF);

	rfm_write(rfmconf, REGFDEVMSB, REGFDEVMSB_DEF);
	rfm_write(rfmconf, REGFDEVLSB, REGFDEVLSB_DEF);

	rfm_write(rfmconf, REGBITRATEMSB, REGBITRATEMSB_DEF);
	rfm_write(rfmconf, REGBITRATELSB, REGBITRATELSB_DEF);

	rfm_write(rfmconf, REGFRFMSB, REGFRFMSB_DEF);
	rfm_write(rfmconf, REGFRFMID, REGFRFMID_DEF);
	rfm_write(rfmconf, REGFRFLSB, REGFRFLSB_DEF);

	rfm_write(rfmconf, REGAFCCTRL, REGAFCCTRL_DEF);

//	rfm_write(rfmconf, REGLISTEN1, REGLISTEN1_DEF);
//	rfm_write(rfmconf, REGLISTEN2, REGLISTEN2_DEF);
//	rfm_write(rfmconf, REGLISTEN3, REGLISTEN3_DEF);

	rfm_write(rfmconf, REGPALEVEL, REGPALEVEL_DEF);
	rfm_write(rfmconf, REGPARAMP, REGPARAMP_DEF);
	rfm_write(rfmconf, REGOCP, REGOCP_DEF);
	rfm_write(rfmconf, REGLNA, REGLNA_DEF);

	rfm_write(rfmconf, REGRXBW, REGRXBW_DEF);
	rfm_write(rfmconf, REGAFCBW, REGAFCBW_DEF);

//	rfm_write(rfmconf, REGOOKPEAK, REGOOKPEAK_DEF);
//	rfm_write(rfmconf, REGOOKAVG, REGOOKAVG_DEF);
//	rfm_write(rfmconf, REGOOKFIX, REGOOKFIX_DEF);

	rfm_write(rfmconf, REGAFCFEI, REGAFCFEI_DEF);

	rfm_write(rfmconf, REGDIOMAPPING1, REGDIOMAPPING1_DEF);
	rfm_write(rfmconf, REGDIOMAPPING2, REGDIOMAPPING2_DEF);

	rfm_write(rfmconf, REGRSSITHRESH, REGRSSITHRESH_DEF);

	rfm_write(rfmconf, REGPREAMBLEMSB, REGPREAMBLEMSB_DEF);
	rfm_write(rfmconf, REGPREAMBLELSB, REGPREAMBLELSB_DEF);

	rfm_write(rfmconf, REGSYNCCONFIG, REGSYNCCONFIG_DEF);
	rfm_write(rfmconf, REGSYNCVALUE1, REGSYNCVALUE1_DEF);
	rfm_write(rfmconf, REGSYNCVALUE2, REGSYNCVALUE2_DEF);
	rfm_write(rfmconf, REGSYNCVALUE3, REGSYNCVALUE3_DEF);
	rfm_write(rfmconf, REGSYNCVALUE4, REGSYNCVALUE4_DEF);
	rfm_write(rfmconf, REGSYNCVALUE5, REGSYNCVALUE5_DEF);
	rfm_write(rfmconf, REGSYNCVALUE6, REGSYNCVALUE6_DEF);
	rfm_write(rfmconf, REGSYNCVALUE7, REGSYNCVALUE7_DEF);
	rfm_write(rfmconf, REGSYNCVALUE8, REGSYNCVALUE8_DEF);

	rfm_write(rfmconf, REGPACKETCONFIG1, REGPACKETCONFIG1_DEF);
	rfm_write(rfmconf, REGPAYLOADLENGHT, REGPAYLOADLENGHT_DEF);
	rfm_write(rfmconf, REGNODEADRS, REGNODEADRS_DEF);
	rfm_write(rfmconf, REGBROADCASTADRS, REGBROADCASTADRS_DEF);
	rfm_write(rfmconf, REGAUTOMODES, REGAUTOMODES_DEF);

	rfm_write(rfmconf, REGFIFOTHRES, REGFIFOTHRES_DEF);
	rfm_write(rfmconf, REGPACKETCONFIG2, REGPACKETCONFIG2_DEF);

//	rfm_write(rfmconf, REGAESKEY1, REGAESKEY1_DEF);
//	rfm_write(rfmconf, REGAESKEY2, REGAESKEY2_DEF);
//	rfm_write(rfmconf, REGAESKEY3, REGAESKEY3_DEF);
//	rfm_write(rfmconf, REGAESKEY4, REGAESKEY4_DEF);
//	rfm_write(rfmconf, REGAESKEY5, REGAESKEY5_DEF);
//	rfm_write(rfmconf, REGAESKEY6, REGAESKEY6_DEF);
//	rfm_write(rfmconf, REGAESKEY7, REGAESKEY7_DEF);
//	rfm_write(rfmconf, REGAESKEY8, REGAESKEY8_DEF);
//	rfm_write(rfmconf, REGAESKEY9, REGAESKEY9_DEF);
//	rfm_write(rfmconf, REGAESKEY10, REGAESKEY10_DEF);
//	rfm_write(rfmconf, REGAESKEY11, REGAESKEY11_DEF);
//	rfm_write(rfmconf, REGAESKEY12, REGAESKEY12_DEF);
//	rfm_write(rfmconf, REGAESKEY13, REGAESKEY13_DEF);
//	rfm_write(rfmconf, REGAESKEY14, REGAESKEY14_DEF);
//	rfm_write(rfmconf, REGAESKEY15, REGAESKEY15_DEF);
//	rfm_write(rfmconf, REGAESKEY16, REGAESKEY16_DEF);

	rfm_write(rfmconf, REGTESTDAGC, AFC_LOW_BETA_OFF);
	rfm_write(rfmconf, REGIRQFLAGS2, 1<<FIFOOVERRUN);

	for (j=0 ; j<99999 ; ++j);
	//rfm69_receive_start();

	if ( rfm_read(rfmconf, REGRXBW) != REGRXBW_DEF ) // проверка SPI
	{
		rfm69_condition = RFM69_SPI_FAILED;
		return -1;
	}
	else	return 0;
}

int rfm_transmit_start(struct rfmconf *rfmconf, uint8_t packet_size_loc, uint8_t address)
{
	int i;

	packet_size = packet_size_loc;
	if (packet_size > RFM69_BUFFER_SIZE)	return -1;						// проверка допустимого размера пакета

	rfm69_condition = RFM69_TX;
	rfm_write(rfmconf, REGOPMODE, REGOPMODE_DEF | TX_MODE);     // включить передатчик

	rfm_write(rfmconf, REGFIFO, packet_size+1);									// передача размера пакета в FIFO
	rfm_write(rfmconf, REGFIFO, address);												// передача адреса в FIFO
	
	for (i = 0 ; i < packet_size ; ++i)
	{
		rfm_write(rfmconf, REGFIFO, packet_buffer[i]);						// передача данных пакета в FIFO
	}

	return 0;
}

void rfm_receive_start(struct rfmconf *rfmconf)
{
	rfm69_condition = RFM69_RX;
	rfm_write(rfmconf, REGOPMODE, REGOPMODE_DEF | RX_MODE);
}

int rfm_receive_small_packet(struct rfmconf *rfmconf)
{
	int i;

	packet_size = rfm_read(rfmconf, REGFIFO);										// считывание размера пакета
	if (packet_size > RFM69_BUFFER_SIZE)	return -1;						// проверка его размера

	rfm_read(rfmconf, REGFIFO);																	// отбрасывается адрес
	--packet_size;	

	for (i = 0 ; i < packet_size ; ++i)
	{
		packet_buffer[i] = rfm_read(rfmconf, REGFIFO);						// чтение пакета из FIFO
	}

	return packet_size;
}

void rfm_sleep (struct rfmconf *rfmconf)
{
	rfm_write(rfmconf, REGOPMODE, REGOPMODE_DEF | SLEEP_MODE);
	rfm69_condition = RFM69_SLEEP;
}

void rfm_stby (struct rfmconf *rfmconf)
{
	rfm_write(rfmconf, REGOPMODE, REGOPMODE_DEF | STBY_MODE);
	rfm69_condition = RFM69_STBY;
}

void rfm_clear_fifo (struct rfmconf *rfmconf)
{
    int i;
    for(i = 0 ; i < RFM69_BUFFER_SIZE ; ++i)
		{
			rfm_read(rfmconf, REGFIFO);
		}
    rfm_write(rfmconf, REGIRQFLAGS2, 1<<FIFOOVERRUN);
}

