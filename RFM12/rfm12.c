#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include "rfm12.h"


/* Библиотека реализует работу с RFM12 в пакетном режиме. Пакет переменой длины (от 1 байта до значения, указанного в макросе DATA_BYTES_MAX)
    Предусмотрено два режима работы: режим непрерывного приема, и обычный.
    В режиме непрерывного приема постоянно ожидается пакет по радиоканалу, как только пакет принят успешно (совпала контрольная сумма) - прием прерывается, включается статус NEW_PACK, RFM переключается в ждущий режим.
    Если включить передачу пакета - прием прекращается, но после того как пакет передастся - возобновится.

    В обычном режиме приемник всегда находится в ждущем режиме, и после передачи или приема пакета переходит опять таки в ждущий режим.
	@author Михаил Белкин (dltech174@gmail.com)
 */



extern volatile uint8_t channel;
volatile uint8_t rfm_condition = 0, pack_buffer[PACKET_SIZE], pack_counter = 0, pack_size = 0, error = 0, nisr;


ISR(INT1_vect)
{
    ++nisr;

	cli();

	switch (rfm_condition)
	{
        case TRANSMIT_PACK :
        {
            if ( (pack_counter < pack_size) && (pack_counter < PACKET_SIZE) )
                rfm12_write(TX_REGISTER_WRITE | pack_buffer[pack_counter++]);
            else
            {
#ifdef  ALWAYS_RX
                rfm12_write(CONFIGURATION_SETTING);         // чтобы обнулить регистр отправки
                rfm_condition = SLEEP;
                rfm12_wait_pack();
#else
                rfm12_sleep();
#endif
            }
            break;
        }

        case RECEIVE_PACK :
        {
            uint8_t buffer;

            buffer = (uint8_t)(rfm12_write(READ_FIFO));

            if (pack_size == 0)
                pack_size = buffer;                                                 // первый байт определяет размер пакета
            else {if ( (pack_counter < pack_size) && (pack_counter < PACKET_SIZE) )
                pack_buffer[pack_counter++] = buffer;
            else
            {
                uint8_t i;
                uint16_t crc=0xffff;

                pack_size -= 2;
                for(i=0 ; i<pack_size ; ++i)    crc = _crc16_update(crc, pack_buffer[i]);

                if( (((uint8_t)(crc>>8)) != pack_buffer[pack_size]) || (((uint8_t)crc) != pack_buffer[pack_size+1]) )  //проверка crc
                {
#ifdef  ALWAYS_RX
                    rfm12_write(FIFO_AND_RESET);                                                       // перестать заполнять FIFO
                    pack_counter = pack_size = 0;
                    rfm12_write(FIFO_AND_RESET | FIFO_EN_MASK);
#else
                    rfm12_sleep();
#endif
                }
                else
                {
                    rfm12_sleep();
                    rfm_condition = NEW_PACK;
                }
            }
            }
            break;
        }

        default :
        {
            error = rfm12_sleep();
            break;
        }
	}
    sei();
}

/** 
	@function
	Запись регистра радиомодуля. Осуществляется по шине SPI.
	@param data	- 16 битный регистр
*/

uint16_t rfm12_write(uint16_t data)
{
    uint16_t in_data = 0;

	RFM12_SPI_PORT &= ~(1<<RFM12_EN);

#ifdef PROGRAM_SPI_ENABLE

	uint8_t i;

    RFM12_SPI_PORT &= ~(1<<RFM12_SCK);
	for (i=0; i<16; i++)
	{
		if (data&0x8000)    RFM12_SPI_PORT |= (1<<RFM12_SDI);	              // вывод текущего бита в порт
		else                RFM12_SPI_PORT &= ~(1<<RFM12_SDI);
		data<<=1;

		if (RFM12_SPI_PIN&(1<<RFM12_SDO)) in_data = (in_data<<1) | 0x0001;    // считываение текущего бита с порта
		else                              in_data = in_data<<1;

		RFM12_SPI_PORT |= (1<<RFM12_SCK);                                     // сигнал синхронизации
		asm("nop");                                                           // немного китайского кода
        asm("nop");
		RFM12_SPI_PORT &= ~(1<<RFM12_SCK);
	}
#else

	SPDR = (uint8_t)(data>>8);
	while (!(SPSR<<7));
    in_data = SPDR;

	SPDR = (uint8_t)data;
	while (!(SPSR<<7));

    in_data = in_data<<8;
    data = SPDR;
	in_data += data;

#endif // PROGRAM_SPI_ENABLE

	RFM12_SPI_PORT |= (1<<RFM12_EN);

    return in_data;

}

/** 
	@function
	Инициализация радиомодуля (запись параметров, включение приемника)
*/

void rfm12_init(void)
{
	RFM12_SPI_DDR |= RFM12_SPI_MASK; 					// инициализация SPI
	RFM12_SPI_PORT |= (1<<RFM12_EN);

    GIMSK |= (1<<INT1);

#ifndef PROGRAM_SPI_ENABLE
    SPCR = RFM12_SPCR;
#endif

//    _delay_ms(100);

    rfm12_write(CONFIGURATION_SETTING);
    rfm12_write(FREQUENCY_SETTING);
    rfm12_write(DATA_RATE);
    rfm12_write(RECEIVER_CONTROL);
    rfm12_write(DATA_FILTER);
    rfm12_write(FIFO_AND_RESET);
    rfm12_write(SYNCHRON_PATTERN);
    rfm12_write(AFC_COMMAND);
    rfm12_write(TX_CONFIGURATION);
    rfm12_write(PLL_SETING);
    rfm12_write(LOW_DUTY_CYCLE);
    rfm12_write(WAKEUP_TIMER);
    rfm12_write(0x0000);

    rfm_condition = pack_size = error = 0;
#ifdef ALWAYS_RX
    rfm12_wait_pack();
#else
    rfm12_sleep();
#endif
}

/** 
	@function
	Инициирует передачу пакета, пакет хранится в глобальном буфере @pack_buffer,
	@param nbytes	размер пакета (может быть меньше максимального)
*/

uint8_t rfm12_transmit_pack( uint8_t nbytes )
{
    uint8_t i=0;
    uint16_t crc = 0xffff;

    if (nbytes > DATA_BYTES_MAX)    return 1;

    switch (rfm_condition)
    {
        case TRANSMIT_PACK :
        {
            return 2;
        }
        case RECEIVE_PACK :
        {
            rfm12_write(FIFO_AND_RESET);                                                           // перестать заполнять FIFO
            rfm12_write(POWER_SC);
        }
    }

    rfm_condition = TRANSMIT_PACK;
    pack_counter = 0;

    pack_buffer[i++] = 0xaa;
    pack_buffer[i++] = 0x2d;
    pack_buffer[i++] = SYNCH_WORD;
    pack_buffer[i++] = nbytes+2;
    for ( ; i<(nbytes+HEADER_NBYTES) ; ++i) crc = _crc16_update(crc, pack_buffer[i]);
    pack_buffer[i++] = (uint8_t)(crc>>8);
    pack_buffer[i++] = (uint8_t)crc;
    pack_buffer[i++] = 0xaa;
    pack_buffer[i++] = 0xaa;
    pack_size = i+2;

    rfm12_write(CONFIGURATION_SETTING | TX_REG_MASK);
    rfm12_write(POWER_TX);      // включить передатик

    return 0;
}

/** 
	@function
	Включает приемник, например при переходе из спящего состояния
*/

uint8_t rfm12_wait_pack(void)
{
    switch (rfm_condition)
    {
        case TRANSMIT_PACK :
            return 1;

        case RECEIVE_PACK :
            return 2;
    }

    rfm_condition = RECEIVE_PACK;
    pack_size = pack_counter = 0;

    rfm12_write(FIFO_AND_RESET | FIFO_EN_MASK);
    rfm12_write(POWER_RX);
    return 0;
}

/** 
	@function
	Переход в спящий режим с малымс энергопотреблением (полезно для экономии батареек)
*/

uint8_t rfm12_sleep(void)
{
    uint8_t buffer;

    rfm_condition = SLEEP;
    pack_counter = 0;

    rfm12_write(POWER_SC);
    buffer = rfm12_write(STATUS_READ);
    rfm12_write(FIFO_AND_RESET);            // выкл. и обнулить FIFO
    rfm12_write(CONFIGURATION_SETTING);     // выкл. и обнулить буфер передатчика

    return buffer;
}

/** 
	@function
	Еще один спящий режим, но выходит из него приемник дольше чем из предыдущего
*/

uint8_t rfm12_deep_sleep(void)
{
    uint8_t buffer;

    rfm_condition = SLEEP;
    pack_counter = 0;

    rfm12_write(POWER_SLEEP);
    buffer = rfm12_write(STATUS_READ);
    rfm12_write(FIFO_AND_RESET);            // выкл. и обнулить FIFO
    rfm12_write(CONFIGURATION_SETTING);     // выкл. и обнулить буфер передатчика

    return buffer;
}

/** 
	@function
	Выбор канала из почти стандартной сетки на 433 Мгц
	@param ch номер канала по порядку (1-14)
*/

void rfm12_channel(uint8_t ch)
{
    switch(ch)
    {
    case 1 :
        rfm12_write(FREQ_CH1);
        break;
    case 2 :
        rfm12_write(FREQ_CH2);
        break;
    case 3 :
        rfm12_write(FREQ_CH3);
        break;
    case 4 :
        rfm12_write(FREQ_CH4);
        break;
    case 5 :
        rfm12_write(FREQ_CH5);
        break;
    case 6 :
        rfm12_write(FREQ_CH6);
        break;
    case 7 :
        rfm12_write(FREQ_CH7);
        break;
    case 8 :
        rfm12_write(FREQ_CH8);
        break;
    case 9 :
        rfm12_write(FREQ_CH9);
        break;
    case 10 :
        rfm12_write(FREQ_CH10);
        break;
    case 11 :
        rfm12_write(FREQ_CH11);
        break;
    case 12 :
        rfm12_write(FREQ_CH12);
        break;
    case 13 :
        rfm12_write(FREQ_CH13);
        break;
    case 14 :
        rfm12_write(FREQ_CH14);
        break;
    }
}
