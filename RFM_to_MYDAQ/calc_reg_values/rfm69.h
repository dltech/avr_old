#ifndef RFM69_H_INCLUDED
#define RFM69_H_INCLUDED
// types of modulation
#define	OOK				0x08
#define	FSK				0x00


/*****************************************************************************/
/* 						radiomodule settings								 */

#define	BITRATE					9600		// bitrate in bit/s
#define CARRIER_FREQ			868250000	// carrier frequency in Hz
#define	OUTPUT_POWER			13			// output power in dBm (13 dBm max)

#define RX_BW					65			// bandwidth of the channel filter in kHz, must be at least twice bigger then bitrate
#define RX_BW_AFC				130			// bandwidth of the channel filter in kHz, must be at least twice bigger then bitrate

#define	OCP_CURRENT				95			// current of the over current protection in mA

/// modulation
#define MODUL_TYPE				FSK			// choose the type of modulation (FSK or OOK)

/// FSK parametres
#define DEVIATION				20000		// frequency deviation in Hz
#define RISE_FALL_TIME_FSK		50			//  rise/fall time of ramp up/down in FSK in us (from 10 to 3400)

/// OOK parametres
// for peak demodulator
#define	OOK_PEAK_THRES_STEP		1			// size of each decrement of the RSSI threshold in the OOK demodulator in dB (from 0.5 to 6.0)
#define OOK_PEAK_THRESH_DEC		1			// period of decrement of the RSSI threshold in the OOK demodulator exprecced in times per cheap (from 0.125 to 16)
// for fixed threshold demodulator
#define	OOKFIXEDTHRESH			6				// theshold value in dB

/// Packet handler configuration
#define PREAMBLE				4			// number of bits in the preamble
#define SYNC_WORD_SIZE			4			// number of bytes in sync word (if 0 - sync word is off)
#define	SYNC_WORD				0x00000000753be1ca
#define SYNCTOL					5			// Number of tolerated bit errors in Sync word

#define RX_ADDRESS				0x05
#define BROADCAST_ADDRESS		0xff

#define AUTO_RESTART_RX_DELAY	1			// delay automatic restart Rx in ms

#define RSSI_THRESH				96			// threshold value for RSSI interrupt in dBm
#define	FIFO_THRESHOLD			32			// threshold value for FIFO interrupt in bytes

#define	CUT_OFF_FREQ			4			// cut off frecuency of the DC canceller exprecced in % of RXBW
#define	CUT_OFF_FREQ_AFC		4			// cut off frecuency of the DC canceller exprecced in % of RXBW
/*****************************************************************************/


/*****************************************************************************/
/*							 ?????????????????? 										 */

#define FXOSC	            32000000
#define	FSTEP	            61
#define	PI                  3.14159265359
#define RFM69_BUFFER_SIZE	66
/*****************************************************************************/

#include "rfm69_table.h"
#include "rfm69_registers.h"

/*****************************************************************************/
/* 						Radiomodule register configuration					 */

#define	REGOPMODE_DEF			( 0<<SEQUENCEROFF | 0<<LISTENON | SLEEP_MODE )
#define REGDATAMODUL_DEF		( PACKET_MODE | MODUL_TYPE | GAUSS_BT05 )

#define REGFDEVMSB_DEF			( (FDEV_CALC(DEVIATION) & 0xff00) >> 8 )
#define REGFDEVLSB_DEF			(  FDEV_CALC(DEVIATION) & 0x00ff )

#define REGBITRATEMSB_DEF		( (BITRATE_CALC(BITRATE) & 0xff00) >> 8 )
#define REGBITRATELSB_DEF		(  BITRATE_CALC(BITRATE) & 0x00ff )

#define	REGFRFMSB_DEF			( (FRF_CALC(CARRIER_FREQ) & 0xff0000) >> 16 )
#define	REGFRFMID_DEF			( (FRF_CALC(CARRIER_FREQ) & 0x00ff00) >> 8 )
#define	REGFRFLSB_DEF			(  FRF_CALC(CARRIER_FREQ) & 0x0000ff )

#define	REGAFCCTRL_DEF			( 0<<(AFCLOWBETAON) )

#define REGLISTEN1_DEF			( 0x00 )
#define	REGLISTEN2_DEF			( 0x00 )
#define	REGLISTEN3_DEF			( 0x00 )

#define	REGPALEVEL_DEF			( 1<<PA0ON | 0<<PA1ON | 0<<PA2ON | (OUT_POWER_CALC(OUTPUT_POWER)) )
#define	REGPARAMP_DEF			( PARAMP )
#define	REGOCP_DEF				( 1<<OCPON	| (OCP_CURRENT_CALC(OCP_CURRENT)) )
#define	REGLNA_DEF				( 1<<LNAZIN | LNAGAIN_AUTO )

#define	REGRXBW_DEF				( DCCFREQ | RXBW )
#define	REGAFCBW_DEF			( DCCFREQAFC | RXBWAFC )

// OOK reqisters
#define	REGOOKPEAK_DEF			( 0x00 )
#define	REGOOKAVG_DEF			( 0x00 )
#define	REGOOKFIX_DEF			( 0x00 )

#define	REGAFCFEI_DEF			( (0<<AFCAUTOCLEAR) | (1<<AFCAUTOON) )

#define	REGDIOMAPPING1_DEF		( DIO0MAP0 | DIO1MAP0 | DIO2MAP2 | DIO3MAP2 )
#define	REGDIOMAPPING2_DEF		( DIO5MAP0 | CLKOUT8M )

#define REGRSSITHRESH_DEF		( RSSI_THRESH_CALC(RSSI_THRESH) )

// packet handler registers
#define	REGPREAMBLEMSB_DEF		( (PREAMBLE & 0xff00) >> 8 )
#define REGPREAMBLELSB_DEF		( PREAMBLE & 0x00ff )

#define REGSYNCCONFIG_DEF		( SYNC_WORD_ON | 0<<FIFOFILLCOND | SYNCSIZE_CALC(SYNC_WORD_SIZE) | (SYNCTOL&0x07) )
#define REGSYNCVALUE1_DEF		(  SYNC_WORD & 0x00000000000000ff )
#define REGSYNCVALUE2_DEF		( (SYNC_WORD & 0x000000000000ff00) >> 8 )
#define REGSYNCVALUE3_DEF		( (SYNC_WORD & 0x0000000000ff0000) >> 16 )
#define REGSYNCVALUE4_DEF		( (SYNC_WORD & 0x00000000ff000000) >> 24 )
#define REGSYNCVALUE5_DEF		( (SYNC_WORD & 0x000000ff00000000) >> 32 )
#define REGSYNCVALUE6_DEF		( (SYNC_WORD & 0x0000ff0000000000) >> 40 )
#define REGSYNCVALUE7_DEF		( (SYNC_WORD & 0x00ff000000000000) >> 48 )
#define REGSYNCVALUE8_DEF		( (SYNC_WORD & 0xff00000000000000) >> 56 )

#define REGPACKETCONFIG1_DEF	( 1<<PACKETFORMAT | ENCODING_OFF | 1<<CRCON | 0<<CRCAUTOCLEAROFF | NODE_BROADCAST_ADDR)
#define	REGPAYLOADLENGHT_DEF	( 0xff )
#define	REGNODEADRS_DEF			( RX_ADDRESS )
#define	REGBROADCASTADRS_DEF	( BROADCAST_ADDRESS )

#define REGAUTOMODES_DEF		( 0x00 )

#define REGFIFOTHRES_DEF		( 1<<TXSTARTCOND | (FIFO_THRESHOLD&0x7f) )
#define REGPACKETCONFIG2_DEF	( INTERPACKETRXDELAY | 1<<AUTORXRESTARTON | 0<<AESON )

#define	REGAESKEY1_DEF			( 0x00 )
#define	REGAESKEY2_DEF			( 0x00 )
#define	REGAESKEY3_DEF			( 0x00 )
#define	REGAESKEY4_DEF			( 0x00 )
#define	REGAESKEY5_DEF			( 0x00 )
#define	REGAESKEY6_DEF			( 0x00 )
#define	REGAESKEY7_DEF			( 0x00 )
#define	REGAESKEY8_DEF			( 0x00 )
#define	REGAESKEY9_DEF			( 0x00 )
#define	REGAESKEY10_DEF			( 0x00 )
#define	REGAESKEY11_DEF			( 0x00 )
#define	REGAESKEY12_DEF			( 0x00 )
#define	REGAESKEY13_DEF			( 0x00 )
#define	REGAESKEY14_DEF			( 0x00 )
#define	REGAESKEY15_DEF			( 0x00 )
#define	REGAESKEY16_DEF			( 0x00 )
/*****************************************************************************/


/*****************************************************************************/
/*							STM32 settings									 */

#define	CRCOK_PKSent_Line	EXTI_Line6
#define	FifoLevel_Line		EXTI_Line5
#define	SyncAddr_Line		EXTI_Line7

#define	NSS_Port			GPIOA
#define	NSS_Pin				GPIO_Pin_3

#define EXTI_Port			GPIO_PortSourceGPIOB

#define	EXTI_Pin1			GPIO_PinSource5
#define	EXTI_Pin2			GPIO_PinSource6
#define	EXTI_Pin3			GPIO_PinSource7



/*****************************************************************************/

enum {
	RFM69_SPI_FAILED = 1,
	RFM69_SLEEP,
	RFM69_STBY,
	RFM69_RX,
	RFM69_TX,
	RFM69_NEW_PACK
};

void rfm69_write(FILE *output, uint8_t address, uint8_t data);
uint8_t rfm69_read(FILE *output, uint8_t address);

int rfm69_init(FILE *output);

#endif // RFM69_H_INCLUDED

