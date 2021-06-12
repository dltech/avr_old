#ifndef RFM22_H_INCLUDED
#define RFM22_H_INCLUDED

/** библиотека для RFM23 и RFM22, большинство регистров обвешано макросами. 
	Но нету конфигурации АЦП, и таких фич как ANTENNA DIVERSITY, датчик температуры и wakeup timer. Также нет макросов для настройки амплитудной модуляции. 
	@author Михаил Белкин 
*/



#define RFM22_DDR   DDRB
#define RFM22_PORT  PORTB
#define RFM22_PIN   PINB

#define NIRQ    0
#define SDN     1
#define NSEL    2
#define SDI     3
#define SDO     4
#define SCK     5
#define RFM22_MASK  ( (1<<SDN) | (1<<NSEL) | (1<<SDI) | (1<<SCK) )

#define RFM22_SPCR 		0x51


/** Конфигурация RFM22 **/

#define PREAMBLE_LENGHT_D     24          //длина преамбулы в битах
#define CHANNEL_STEP_KHZ    300

#define CARRIER_FREQ    433.15          // частота несущей в мГц
#define DEVIATION       75              // частота девиации в кГц
#define DATA_RATE       115.2           // битрейт в кбит/с

#define DATA_BYTES_MAX  24
#define BUFFER_SIZE_MAX DATA_BYTES_MAX + 2

#define RFM22_WAIT_TIME 50

#define DEVICE_TYPE_P     0x00
#define DEVICE_VER_P      0x01
#define DEVICE_STATUS_P   0x02

#define INTERRUPT_ST1_P   0x03
#define INTERRUPT_EN1_P   0x05
#define FIFO_OVERFLOW   7
#define TX_FIFO_FULL    6
#define TX_FIFO_ENPTY   5
#define RX_FIFO_FULL    4
#define EXTERNAL_INT    3
#define PACKET_SENT     2
#define PACKET_RECEIVED 1
#define CRC_ERROR       0

#define INTERRUPT_ST2_P   0x04
#define INTERRUPT_EN2_P   0x06
#define SYNC_WORD_DET     7
#define VALID_PR_DET      6
#define INVALID_PR_DET    5
#define RSSI_INT          4
#define WAKE_UP_TIMER_INT 3
#define LOW_BATTERY_INT   2
#define CHIP_READY        1
#define POR_INT           0

#define OPERATING_MODE1_P 0x07
#define RESET           7
#define LOW_BATTERY_EN  6
#define WAKE_UP_TIM_EN  5
#define OSC_32RHZ_EN    4
#define TX_ON           3
#define RX_ON           2
#define PLL_ON          1
#define READY_MODE      0
#define STANDBY         0x00


#define OPERATING_MODE2_P 0x08
#define MULTI_PACK      4
#define AUTOTX          3
#define LDC_MODE        2
#define RX_FIFO_CLR     1
#define TX_FIFO_CLR     0

#define OSC_LOAD_CAP_P    0x09

#define MCU_CLK_P         0x0a
#define TAIL_0          0
#define TAIL_128        1
#define TAIL_256        2
#define TAIL_512        3

#define LOW_FREQ_EN     3       // вкл генератор на 32768 во время спячки (если 0 то в спящем режиме ничего не генерируется)

#define MCU_30MHZ       0
#define MCU_15MHZ       1
#define MCU_10MHZ       2
#define MCU_4MHZ        3
#define MCU_3MHZ        4
#define MCU_1MHZ        5
#define MCU_32KHZ       6

#define GPIO0_CONF_P      0x0b
#define GPIO1_CONF_P      0x0c
#define GPIO2_CONF_P      0x0d


#define LOW_BATTERY_THR_P 0x1a
#define BAT_THRES(volt) ((volt-1.7)/0.05)

#define IF_FILTER_BW_P    0x1c
#define AFC_GEAR_OVER_P   0x1d
#define AFC_TIMING_CTRL_P 0x1e
#define CLKREC_GEAROVER_P 0x1f
#define CLK_REC_OVER_P    0x20
#define CLK_REC_OFFSET2_P 0x21
#define CLK_REC_OFFSET1_P 0x22
#define CLK_REC_OFFSET0_P 0x23
#define CLK_REC_CRGAIN1_P 0x24
#define CLK_REC_CRGAIN0_P 0x25

#define RSSI_P            0x26
#define RSSI_THRESHOLD_P  0x27

#define AFC_LIMITER_P     0x2a

#define DATA_ACC_CTRL_P   0x30
#define ENPACRX         7       // вкл прием пакетов
#define LSBFRST         6
#define CRCDONLY        5       // контрольная сумма только над данными, если 0 - над всем пакетом
#define ENPACTX         3       // вкл передачу пакетов
#define ENCRC           2       // вкл контрольную сумму

#define CCIT            0       // разновидности контрольных сумм
#define CRC16           1
#define IEC16           2
#define BIACHEVA        3

#define EZMAC_STATUS_P    0x31  // регистр, хранящий текущий статус устройства
#define RXCRC1          6
#define PKSRCH          5   // RFM ожидает пакет
#define PKRX            4   // RFM принимает пакет
#define PKVALID         3   // пакет успешно принят
#define CRCERROR        2   // ошибка контрольной сумы
#define PKTX            1   // пакет передается в данный момент
#define PKSENT          0   // пакет успешно отправлен

#define HEADER_CTRL1_P    0x32
#define NO_BROADCAST    0   // широковещательный адрес, для каких байт заголовка применим
#define BROADCAST_EN0   1
#define BROADCAST_EN1   2
#define BROADCAST_EN01  2
//...
#define NO_HEAD_CHECK   0   // какие байты заголовка являются адресами (проверяются)
#define HEAD_CHECK_EN0  0
#define HEAD_CHECK_EN1  0
#define HEAD_CHECK_EN01 0
//...

#define HEADER_CTRL2_P    0x33
#define SKIPSYN         7   // какаято хрень с поиском синхронизационного слова (лучше этот бит не выставлять)

#define NO_HEADER       0   // размер заголовка
#define HEADER_3        1
#define HEADER_32       2
#define HEADER_321      3
#define HEADER_3210     4

#define FIXPKLEN        3   // зафиксировать длину пакета (иначе переменная)

#define SYNC_WORD_3     0
#define SYNC_WORD_32    1   // количество байт синхронизационного слова
#define SYNC_WORD_321   2
#define SYNC_WORD_3210  3

#define PREAMBLE_LENGHT_P 0x34
#define PREAMBLE_DETCTR_P 0x35
#define SYNCH_WORD3_P     0x36
#define SYNCH_WORD2_P     0x37
#define SYNCH_WORD1_P     0x38
#define SYNCH_WORD0_P     0x39
#define TX_HEADER3_P      0x3a
#define TX_HEADER2_P      0x3b
#define TX_HEADER1_P      0x3c
#define TX_HEADER0_P      0x3d
#define TX_PACK_LENGHT_P  0x3e
#define CHECK_HEADER3_P   0x3f
#define CHECK_HEADER2_P   0x40
#define CHECK_HEADER1_P   0x41
#define CHECK_HEADER0_P   0x42
#define HEADER_EN3_P      0x43
#define HEADER_EN2_P      0x44
#define HEADER_EN1_P      0x45
#define HEADER_EN0_P      0x46
#define RX_HEADER3_P      0x47
#define RX_HEADER2_P      0x48
#define RX_HEADER1_P      0x49
#define RX_HEADER0_P      0x4a
#define RX_PACK_LENGHT_P  0x4b
#define INV_PREAMB_THR_P  0x60

#define OSC_POR_CTRL_P    0x61
#define CLKHYST         4
#define ENBIAS2X        3
#define ENAMP2X         2
#define BUFOUR          1   // если 1 - TX буффер контролируется следующим битом, иначе - автоматически
#define ENBUF           0

#define TX_POWER_P        0x62
#define LNA_SW          3

#define TX_POW_M8       0   // мощность передатчика (M типо минус)
#define TX_POW_M5       1
#define TX_POW_M2       2
#define TX_POW_1        3
#define TX_POW_4        4
#define TX_POW_7        5
#define TX_POW_10       6
#define TX_POW_13       7

#define TX_DATA_RATE1_P   0x6e
#define TX_DATA_RATE0_P   0x6f
#define TXDTRTSCALE_EN  ( (DATA_RATE) < (30) ? (0) : (1) )
#define TXDR            ( (DATA_RATE) < (30) ? ((DATA_RATE*2097152)/1000) : ((DATA_RATE*65536)/1000) )

#define MODULAT_CTRL1_P   0x70
#define TXDTRTSCALE     5   // если битрейт ниже 30 кбод установить этот бит
#define ENPHPWDN        4   // выкл обаботчик пакетов, если
#define MANPPOL         3   // передает 1 вменсто 0 и наоборот, приемника не касается
#define ENMANINV        2   // инверсия manchester encoding 1 если вкл
#define ENMANCH         1   // включает manchester encoding
#define ENWHITE         0   // включает data whitening

#define MODULAT_CTRL2_P   0x71
#define DIRECTM_GPIO    0
#define DIRECTM_SDI     1
#define FIFO_MODE       2
#define PN9_MODE        3

#define ENINV           3   // подставной бит (инвертирует данные)

#define WITHOUT_MOD     0
#define OOK             1
#define FSK             2
#define GFSK            3

#define FREQ_DEVIATION_P  0x72
#define FREQDIV    (DEVIATION*1000/625)

#define FREQ_BAND_SEL_P   0x75
#define HBSEL       5
#define HBSEL_EN   ( (CARRIER_FREQ) >= (480) ? (1) : (0) )
#define FREQBAND   ((CARRIER_FREQ/(10*(HBSEL_EN+1))) - 24)

#define CARRIER_FREQ1_P   0x76
#define CARRIER_FREQ2_P   0x77
#define CARRIER     ( ( (CARRIER_FREQ/(10*(HBSEL_EN+1))) - ((uint16_t)(CARRIER_FREQ/(10*(HBSEL_EN+1)))) ) * 64000 )

#define FREQ_HOP_CHSEL_P  0x79
#define FREQ_HOP_STSIZE_P 0x7a
#define TX_FIFO_CTRL1_P   0x7c
#define TX_FIFO_CTRL2_P   0x7d
#define RX_FIFO_CTRL_P    0x7e
#define FIFO_ACCESS_P     0x7f


/** Тут задаются значения регистров для инициализации **/

#define INTERRUPT_EN1   ((1<<PACKET_SENT) | (1<<PACKET_RECEIVED))
#define INTERRUPT_EN2   ()
#define OPERATING_MODE1 ()
#define OPERATING_MODE2 ()
#define OSC_LOAD_CAP    ()
#define MCU_CLK         (0x00)
#define GPIO1_CONF      (0x00)
#define GPIO2_CONF      (0x00)
#define GPIO3_CONF      (0x00)
#define LOW_BATTERY_THR ((uint8_t)BAT_THRES(2.5)&0x1f)

// эти регистры берем из файлика, ибо вломы формулы набирать
#define IF_FILTER_BW    (0x83)
#define AFC_GEAR_OVER   ()
#define AFC_TIMING_CTRL ()
#define CLKREC_GEAROVER ()
#define CLK_REC_OVER    (0x68)
#define CLK_REC_OFFSET2 (0x01)
#define CLK_REC_OFFSET1 (0x3a)
#define CLK_REC_OFFSET0 (0x93)
#define CLK_REC_CRGAIN1 (0x13)
#define CLK_REC_CRGAIN0 (0xca)

#define RSSI_THRESHOLD  (0xaa)

// опять из файлика
#define AFC_LIMITER     (0x50)

#define DATA_ACC_CTRL   ( (1<<ENPACRX) | (1<<ENPACTX) )
#define HEADER_CTRL1    ((NO_BROADCAST<<4) | NO_HEAD_CHECK )
#define HEADER_CTRL2    ( (NO_HEADER<<4) | (SYNC_WORD_32<<1) | (((PREAMBLE_LENGHT/4)&0x100)>>8) )
#define PREAMBLE_LENGHT ((uint8_t)(PREAMBLE_LENGHT_D/4))
#define PREAMBLE_DETCTR ( (((PREAMBLE_LENGHT/8)&0x1f)<<3) )
#define SYNCH_WORD3     (0x2d)
#define SYNCH_WORD2     (0xd4)
#define SYNCH_WORD1     (0x00)
#define SYNCH_WORD0     (0x00)
#define TX_HEADER3      (0x00)
#define TX_HEADER2      (0x00)
#define TX_HEADER1      (0x00)
#define TX_HEADER0      (0x00)
#define TX_PACK_LENGHT  (0x00)
#define HEADER_EN3      (0xff)               // маска адреса
#define HEADER_EN2      (0xff)
#define HEADER_EN1      (0xff)
#define HEADER_EN0      (0xff)
#define INV_PREAMB_THR  ( (20/4)<<4 )
#define OSC_POR_CTRL    (0x04)
#define TX_POWER        ( (1<<LNA_SW) | TX_POW_13 )
#define TX_DATA_RATE1   (0x1d)                   // брать из файлика
#define TX_DATA_RATE0   (0x7e)                   // брать из файлика
#define MODULAT_CTRL1   (TXDTRTSCALE_EN<<TXDTRTSCALE)
#define MODULAT_CTRL2   ( (FIFO_MODE<<4) | (((uint16_t)(FREQDIV)&0x100)<<2) | FSK )
#define FREQ_DEVIATION  (0x78)                   // брать из файлика
#define FREQ_BAND_SEL   ( (HBSEL_EN<<HBSEL) | ((uint8_t)FREQBAND&0x1f) )
#define CARRIER_FREQ1   (((uint16_t)CARRIER&0xff00)>>8)
#define CARRIER_FREQ2   ((uint8_t)CARRIER)
#define FREQ_HOP_CHSEL  (0x00)
#define FREQ_HOP_STSIZE (CHANNEL_STEP_KHZ/10)
#define TX_FIFO_CTRL1   (0x00&0x3f)           // нужно указать порог почти полного FIFO
#define TX_FIFO_CTRL2   (0x00&0x3f)           // нужно указать порог почти пустого FIFO
#define RX_FIFO_CTRL    (0x00&0x3f)           // нужно указать порог почти полного FIFO

volatile uint8_t pack_buffer[BUFFER_SIZE_MAX], pack_size;


/* чтение и запись по SPI */
void rfm22_write(uint8_t address,uint8_t data);         // записать один байт, передается адрес и данные
uint8_t rfm22_read(uint8_t address);                    // считать один байт, передается адрес, возвращаются данные

void rfm22_write_burst(uint8_t address, volatile uint8_t *data, uint8_t lenght);    // записать несколько байт, передается адрес, указатель на массив и длина
void rfm22_read_burst(uint8_t address, volatile uint8_t *data, uint8_t lenght);     // считать несколько байт, передается адрес, указатель на массив с данными и длина

/* Инициализация RFM. Инициализация интерфейса и запись конфигурационных регистров. Возвращает 1 если связи нет.*/
uint8_t rfm22_init(void);

/* Функции приема и передачи пакета */
void rfm22_transmit_start(uint8_t nbytes);              // начало передачи пакета, функции передается размер пакета в байтах
uint8_t rfm22_transmit_complete(void);                  // окончание передачи, если пакет принят успешно - возвращает 0.

void rfm22_receive_start(void);                         // начало приема пакета, включает передатчик
uint8_t rfm22_receive_complete(void);                   // окончание приема, если пакет принеят успешно - выключает приемник, и переходит в спящий режим

/* Служебные функцции (пользователю нафиг не нужны). */
void rfm22_sleep(void);                                 // чистит буферы и уводит контроллер в спячку
void rfm22_allclean(void);                              // обнуляет быферы и прерывания
uint8_t rfm22_wait_event(uint8_t event);               // ждет событие, если произошло - возвращает последний считанный регистр прерываний, иначе выходит по таймауту, и возвращает 0.

#endif // RFM22_H_INCLUDED




/*
#define RX_BW2_6KHZ     0x51
#define RX_BW2_8KHZ     0x52
#define RX_BW3_1KHZ     0x53
#define RX_BW3_2KHZ     0x54
#define RX_BW3_7KHZ     0x55
#define RX_BW4_2KHZ     0x56
#define RX_BW4_5KHZ     0x57
#define RX_BW4_9KHZ     0x41
#define RX_BW5_4KHZ     0x42
#define RX_BW5_9KHZ     0x43
#define RX_BW6_1KHZ     0x44
#define RX_BW7_2KHZ     0x45
#define RX_BW8_2KHZ     0x46
#define RX_BW8_8KHZ     0x47
#define RX_BW9_5KHZ     0x31
#define RX_BW10_6KHZ    0x32
#define RX_BW11_5KHZ    0x33
#define RX_BW12_1KHZ    0x34
#define RX_BW14_2KHZ    0x35
#define RX_BW16_2KHZ    0x36
#define RX_BW17_5KHZ    0x37
#define RX_BW18_9KHZ    0x21
#define RX_BW21_0KHZ    0x22
#define RX_BW22_7KHZ    0x23
#define RX_BW24_0KHZ    0x24
#define RX_BW28_2KHZ    0x25
#define RX_BW32_2KHZ    0x26
#define RX_BW34_7KHZ    0x27
#define RX_BW37_7KHZ    0x11
#define RX_BW41_7KHZ    0x12
#define RX_BW45_2KHZ    0x13
#define RX_BW47_9KHZ    0x14
#define RX_BW56_2KHZ    0x15
#define RX_BW64_1KHZ    0x16
#define RX_BW69_2KHZ    0x17
#define RX_BW75_2KHZ    0x01
#define RX_BW83_2KHZ    0x02
#define RX_BW90_0KHZ    0x03
#define RX_BW95_3KHZ    0x04
#define RX_BW112_1KHZ   0x05
#define RX_BW127_9KHZ   0x06
#define RX_BW137_9KHZ   0x07
#define RX_BW142_8KHZ   0x94
#define RX_BW167_8KHZ   0x95
#define RX_BW181_1KHZ   0x99
#define RX_BW191_5KHZ   0x8f
#define RX_BW225_1KHZ   0x81
#define RX_BW248_8KHZ   0x82
#define RX_BW269_3KHZ   0x83
#define RX_BW248_9KHZ   0x84
#define RX_BW335_5KHZ   0x88
#define RX_BW361_8KHZ   0x89
#define RX_BW420_2KHZ   0x8a
#define RX_BW468_4KHZ   0x8b
#define RX_BW518_8KHZ   0x8c
#define RX_BW577_0KHZ   0x8d
#define RX_BW620_7KHZ   0x8e

#define AFC_WIDEBAND    7
#define AFC_EN          6
#define AFC_HIGH_GEAR(c)    (c&0x07)   // определяет обратную связь подстройки частоты, чем больше, тем слабее, и тем длиннее должна быть преамбула (дефолтное 0)
#define AFC_BYPASS      2              // 1 если выключена, 0 если включена
#define PH0SIZE         0              // 0 - сброс преамбулы после 5 нулей, 1 - после 3

#define SHWAIT(c)       (c&0x07)       // длительность цикла AFC, при значении в 1 - 4 бита

#define OVERSAMPLING_RATE   (500*(1+2*(IF_FILTER_BW&0x80)) / ((1<<(((IF_FILTER_BW&0x70)>>4)-3))*BAUDRATE*(1+ENMACH))
#define NCO_OFFSET          ( BAUDRATE * (1+ENMANCH) * (1<<(20+((IF_FILTER_BW&0x70)>>4))) / 500 / (1+2*((IF_FILTER_BW&0x80)>>7)) )
#define CR_GAIN             (2 + (0x10000*(1+ENMANCH)*BAUDRATE)/ )
#define IF_FILTER_BW    (RX_BW269_3KHZ)
#define AFC_LOOP_GEAR   ((1<<AFC_EN) | (AFC_GEAR<<3) | (1<<AFC_BYPASS))
#define AFC_TIMING      (SHWAIT(1)<<3)
#define CLK_REC_GEAR    (0x03)
#define CLK_REC_OSR     ((uint8_t)OVERSAMPLING_RATE)
#define CLK_REC_OFF2    (((OVERSAMPLING_RATE&0x700)>>3) | ((NCO_OFFSET&0xf0000)>>16))
#define CLK_REC_OFF1    ((NCO_OFFSET&0x0ff00)>>8)
#define CLK_REC_OFF0    (NCO_OFFSET&0x000ff)
#define CLK_REC_GAIN    ()
*/
