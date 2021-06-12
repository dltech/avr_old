#ifndef RFM12_H_INCLUDED
#define RFM12_H_INCLUDED

/**
	@author Михаил Белкин (dltech174@gmail.com) 
*/

//#define ALWAYS_RX
#define PROGRAM_SPI_ENABLE

#define SYNCH_WORD  0xd4

// Максимальный размер пакета
#define DATA_BYTES_MAX  24
#define HEADER_NBYTES   4
#define NCHANNELS       14

#define PACKET_SIZE     HEADER_NBYTES + DATA_BYTES_MAX + 4


enum
{
    TRANSMIT_PACK = 1,
    RECEIVE_PACK,
    SLEEP,
    NEW_PACK
};


/** настройки портов и SPI**/

#define RFM12_SPI_PORT  PORTB
#define RFM12_SPI_PIN   PINB
#define RFM12_SPI_DDR   DDRB

#define RFM12_SDI		5
#define RFM12_SDO		6
#define RFM12_SCK		7
#define RFM12_EN		4
#define RFM12_SPI_MASK  ((1<<RFM12_SDI) | (1<<RFM12_SCK) | (1<<RFM12_EN))

#define RFM12_SPCR      0x51




/** настройки RFM **/

#define BAND433     1
#define BAND868     2
#define BAND915     3

#define TX_REG_MASK         0x0080
#define FIFO_MODE_MASK      0x0040
#define FIFO_EN_MASK        0x0002

#define CRYSTAL_CAP(cap)    ((cap-8.5)/0.5)

#define CARRIER_FREQ_433(freq)  ((freq - 430)*400)
#define CARRIER_FREQ_868(freq)  ((freq - 860)*200)
#define CARRIER_FREQ_915(freq)  ((freq - 900)*133.3)

#define BAUDRATE_SLOW(br)       (0x80 | ((10000./29./8./br)-1))      // для скоростей ниже 2,7Кбод
#define BAUDRATE_FAST(br)       ((10000./29./br)-1)                 // для скоростей выше 2,7Кбод

#define RX_BW400k   1
#define RX_BW340k   2
#define RX_BW270k   3
#define RX_BW200k   4
#define RX_BW134k   5
#define RX_BW67k    6

#define LNA_0		0
#define LNA_6		1
#define LNA_14		2
#define LNA_20		3

#define RSSI_103	0
#define RSSI_97		1
#define RSSI_91		2
#define RSSI_85		3
#define RSSI_79		4
#define RSSI_73		5
#define RSSI_67		6
#define	RSSI_61		7

#define CLOCK_RECOVERY_AUTO     1
#define CLOCK_RECOVERY_MAN      0
#define CLOCK_RECOVERY_FAST     1
#define CLOCK_RECOVERY_SLOW     0
#define DQD(par)                (par)

#define FIFO_THETHOLD(bytes)    (bytes)
#define SENSITIVE_RESET         0
#define NON_SENS_RESET          1

#define AFC_ONE_DEV_MODE        3
#define AFC_MULTIPLE_DEV_MODE   2

#define AFC_LIMIT_INF           0
#define AFC_LIMIT_15            1
#define AFC_LIMIT_7             2
#define AFC_LIMIT_3             3

#define AFC_FINE_MODE           1
#define AFC_NFINE_MODE          0

#define FSK_INVERSION_ON        1
#define FSK_INVERSION_OFF       0

#define TX_BW15k    0
#define TX_BW30k    1
#define TX_BW45k	2
#define TX_BW60k    3
#define TX_BW75k    4
#define TX_BW90k    5
#define TX_BW105k   6
#define TX_BW120k   7

#define PWR_0DB		0
#define PWR_3DB		1
#define PWR_6DB		2
#define PWR_9DB		3
#define PWR_12DB	4
#define PWR_15DB	5
#define PWR_18DB	6
#define PWR_21DB	7

#define PH_DETECTOR_DELAY_ON    1
#define PH_DETECTOR_DELAY_OFF   0

#define PLL_DITHERING_ON        0
#define PLL_DITHERING_OFF       1

#define PLL_BW_BR256K           1
#define PLL_BW_BR86K            0

#define LDC_ON                  1
#define LDC_OFF                 0

#define DETECT_VOLTADE(volt)    ((volt - 2.25)/0.1)

// конфигурация радиомодема происходит здесь
#define CONFIGURATION_SETTING   (0x8040 | (BAND433<<4) | ((uint8_t)CRYSTAL_CAP(12)&0x0f))
#define POWER_SLEEP             (0x8201)
#define POWER_TX                (0x8238)
#define POWER_RX                (0x82c8)
#define POWER_SC                (0x8218)
#define FREQUENCY_SETTING       (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.11)&0x0fff))
#define DATA_RATE               (0xc600 | ((35) & 0x00ff))
#define RECEIVER_CONTROL        (0x9400 | (RX_BW134k<<5) | (LNA_6<<3) | RSSI_97)
#define DATA_FILTER             (0xc228 | (CLOCK_RECOVERY_MAN<<7) | (CLOCK_RECOVERY_SLOW<<6) | (DQD(4)&0x07))
#define FIFO_AND_RESET          (0xca00 | (FIFO_THETHOLD(8)<<4) | NON_SENS_RESET)
#define SYNCHRON_PATTERN        (0xce00 | (SYNCH_WORD&0x00ff))
#define READ_FIFO               (0xb000)
#define AFC_COMMAND             (0xc403 | (AFC_MULTIPLE_DEV_MODE<<6) | (AFC_LIMIT_7<<4) | (AFC_FINE_MODE<<2))
#define TX_CONFIGURATION        (0x9800 | (FSK_INVERSION_OFF<<8) | (TX_BW30k<<4) | PWR_0DB)
#define PLL_SETING              (0xcc72 | (PH_DETECTOR_DELAY_OFF<<3) | (PLL_DITHERING_OFF<<2) | PLL_BW_BR256K)
#define TX_REGISTER_WRITE       (0xb800)
#define WAKEUP_TIMER            (0xe000)
#define LOW_DUTY_CYCLE          (0xc800 | LDC_OFF)
#define LOW_BATT_AND_MCU_CLK    (0xc000 | (DETECT_VOLTADE(2.65)&0x0f))
#define STATUS_READ             (0x0000)

#define FREQ_CH1    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.11)&0x0fff))
#define FREQ_CH2    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.23)&0x0fff))
#define FREQ_CH3    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.35)&0x0fff))
#define FREQ_CH4    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.47)&0x0fff))
#define FREQ_CH5    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.59)&0x0fff))
#define FREQ_CH6    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.71)&0x0fff))
#define FREQ_CH7    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.83)&0x0fff))
#define FREQ_CH8    (0xA000 | ((uint16_t)CARRIER_FREQ_433(433.95)&0x0fff))
#define FREQ_CH9    (0xA000 | ((uint16_t)CARRIER_FREQ_433(434.07)&0x0fff))
#define FREQ_CH10   (0xA000 | ((uint16_t)CARRIER_FREQ_433(434.19)&0x0fff))
#define FREQ_CH11   (0xA000 | ((uint16_t)CARRIER_FREQ_433(434.31)&0x0fff))
#define FREQ_CH12   (0xA000 | ((uint16_t)CARRIER_FREQ_433(434.43)&0x0fff))
#define FREQ_CH13   (0xA000 | ((uint16_t)CARRIER_FREQ_433(434.55)&0x0fff))
#define FREQ_CH14   (0xA000 | ((uint16_t)CARRIER_FREQ_433(434.67)&0x0fff))



/* Функция передачи данных по spi */
uint16_t rfm12_write(uint16_t data);

/* инициализация */
void rfm12_init(void);
void rfm12_channel(uint8_t ch);

/* начать передачу или прием пакета */
uint8_t rfm12_transmit_pack(uint8_t lenght);
uint8_t rfm12_wait_pack(void);

/* спящий режим */
uint8_t rfm12_sleep(void);          // режим спячки - приемник и передатчик выключены, но тактовый генератор работает
uint8_t rfm12_deep_sleep(void);     // режим глубокой спячки - все выключено


#endif // RFM12_H_INCLUDED
