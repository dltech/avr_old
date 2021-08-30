#include <avr/io.h>
#include <util/delay.h>
#include "modem.h"
#include "rfm_22_23.h"



/* Считывает последовательно lenght байт по SPI, начиная с адреса address, в *data */

void  rfm_22_read_burst  (uint8_t address, volatile uint8_t *data, uint8_t lenght) {

	register uint8_t i=0;

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address;		 			// записывает адрес
	while (!(SPSR<<7));

	for(i=0 ; i<lenght ; ++i) {		// считывает данные
		SPDR=0xff;
		while (!(SPSR<<7));
		*(data+i) = SPDR;
		}

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;

	return;
	}




/* Записывает последовательно lenght байт по SPI из *data по адресу address */

void rfm_22_write_burst (uint8_t address, volatile uint8_t *data, uint8_t lenght) {

	register uint8_t i=0;

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address | (1 << 7);		 // записывает адрес

	while(!(SPSR<<7));

	for(i=0 ; i<lenght ; ++i) {		// записывает данные
		SPDR = *(data+i);
		while (!(SPSR<<7));
		}

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;

	return;
	}



/* Записывает по SPI data по адресу address */

void rfm_22_write (unsigned char address,unsigned char data) {

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address | (1<<7);		 // записывает адрес
	while (!(SPSR<<7));

	SPDR = data;         			// записывает данные
	while (!(SPSR<<7));

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;

	return;
	}




/* Считывает по SPI данные data по адресу address */

uint8_t rfm_22_read(unsigned char address) {

	unsigned char data;

	RFM_22_SPI_EN_PORT &= ~RFM_22_SPI_EN_PIN;

	SPDR = address;		 		// записывает адрес
	while (!(SPSR<<7));

	SPDR = 0xff;         		// записывает пустые данные

	while (!(SPSR<<7));

	data = SPDR;

	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;

	return data;
	}




/* Записывает все необходимые конфигурационные регистры в RFM, проверяет связь с RFM. Если RFM не отвечает - возвращает 1, иначе - 0 */

uint8_t rfm_22_init(void) {

uint8_t		init_table_io[3] = {0x53, 0x5c, 0x56},
            init_table_freq[6] = {0x5e, 0x01, 0x5d, 0x86, 0x03, 0x7e},
            init_table_ph[4] = {0x11, 0x46, 0x0a, 0x20},
            init_table_synch[4] = {0x28, 0x15, 0x23, 0x42},
            init_table_mod[3] = {0x0c, 0x27, 0xe0},
            init_table_freqdiv[3] = {0x73, 0x69, 0x00};

	uint8_t error;

	RFM_22_SDN_DDR |= RFM_22_SDN_EN;					//включает RFM
	RFM_22_SDN_PORT &= ~RFM_22_SDN_EN;

	RFM_22_SPI_DDR |= RFM_22_SPI_PIN; 					//инициализация SPI
	RFM_22_SPI_EN_DDR |= RFM_22_SPI_EN_PIN;
	RFM_22_SPI_EN_PORT |= RFM_22_SPI_EN_PIN;
	SPCR = RFM_22_SPCR;

	rfm_22_write(0x07,0x00); 							// режим standby

//    _delay_ms(10);                                     // ждет пока RFM включится

	rfm_22_write(0x05,0x37);							// включает прерывания
	rfm_22_write(0x06,0x00);

	rfm_22_read(0x03);						 			// обнуляет прерывания
	rfm_22_read(0x04);

	rfm_22_write(0x09,0x7f);							// емкость 12,5пФ

	rfm_22_write_burst(0x0b, init_table_io, 3);			// порты вода-вывода

	rfm_22_write(0x6d,0x1f);    						// мощность 13dBm

	rfm_22_write(0x1c,0x95);
	rfm_22_write(0x1d,0x40);

	rfm_22_write_burst(0x20, init_table_freq, 6);		// частоты

	rfm_22_write(0x27,0x1e);							// порог RSSI

	rfm_22_write(0x30, 0x8d);

	rfm_22_write_burst(0x32, init_table_ph, 4);			// обработчик пакетов

	rfm_22_write_burst(0x36, init_table_synch, 4);		// синхронизация

	rfm_22_write(0x3a, device_address);					// адрес возврата
	rfm_22_write(0x3d, connect_address);
	rfm_22_write(0x42, device_address);					// адрес устройства
	rfm_22_write(0x46,0xff);							// маска адреса



	rfm_22_write(0x6e,0x20);							// скорость передачи
	rfm_22_write(0x6f,0xc5);

	rfm_22_write_burst(0x70, init_table_mod, 3);		// модуляция

	rfm_22_write_burst(0x75, init_table_freqdiv, 3);	// конфигурация несущей частоты и девиации

    rfm_22_write(0x7d,0x0a);                            // нижний порог fifo передатчика
	rfm_22_write(0x7e,0x00);							// порог заполнения fifo приемника

	error=rfm_22_read(0x20);

	if (  *init_table_freq != error ) error=1;			// проверка модуля
	else error=0;

	return error;
	}




/* Обнуляет FIFO и прерывания RFM, включая режим приемника */

void rfm_22_clear(void) {

    rfm_22_write(0x07,0x05);            // переключает RFM в режим приемника

    rfm_22_write(0x7e,0x00);            // Порог заполнения FIFO 0.

    rfm_22_write(0x08,0x03);			// стирает fifo RFM
    rfm_22_write(0x08,0x00);

    rfm_22_read(0x03);                  // обнуляет прерывания
    rfm_22_read(0x04);

	return;
	}




/* Функция ожидания определенного события. Ждет пока не сработает определенное прерывание в RFM. В качестве аргумента передается ожидаемый регистр флагов прерываний RFM.
    Если событие не произошло - срабатывает таймаут. Возвращает последний считанный регистр флагов или ноль (если ничего не происходило).   */

uint8_t rfm_22_wait_event (uint8_t event) {

    uint8_t interrupt = 0;

    TCNT1 = 0;

    while (((interrupt & event) == 0) & (TCNT1<RFM_22_WAIT_TIME))
        if ((RFM_22_NIRQ_PIN & RFM_22_NIRQ_EN) == 0)
            interrupt = rfm_22_read(0x03);

    return interrupt;
    }




/* Функция для передачи команд. Передает пакет состоящий из заголовка и одного байта. Задержка, связанная с ожиданием передачи пакета учтена.
    В качестве аргумента передается команда.    */

void rfm_22_transmit_command(uint8_t command) {

    rfm_22_write(0x3e,0x01);						// костыль из за долбанутых настроек порта RFM
    rfm_22_write(0x7f,0xee);

    rfm_22_write(0x3b,command);

	rfm_22_write(0x07,0x09);					    // вкл режим передатчика

//	delay_timer(20);								// задержка, чтобы при передаче пакета не беспокоить RFM

	return;
	}




/* Специальная функция для передачи пакета в 256 байт. 256й байт пихается в заголовок. */

void rfm_22_transmit_packet_256(void) {

    --npack;
    rfm_22_write(0x3a,buffer[255]);

    rfm_22_transmit_packet();                       // передает остальной пакет

    return;
	}




/* Отправляет пакет длиной от 1 до 255 байт. Пакет считывается из буфера, размер пакета определяется глобальной переменной npack.
    Задержка, связанная с ожиданием передачи пакета учтена. Также подразумевается что первые 64 байта уже есть в fifo. */

void rfm_22_transmit_packet(void) {

    uint8_t nbyte = npack;

    rfm_22_write(0x3e,nbyte);						            // размер пакета данных в RFM
    rfm_22_write(0x07,0x09);						            // вкл режим передатчика

    if ( nbyte>64 ) {
        nbyte -= 64;                                            // корректирует количество оставшихся байт в пакете

        while (nbyte>32) {

            rfm_22_wait_event(0x20);                            // ждет пока FIFO опустеет
            rfm_22_write_burst(0x7f,buffer+npack-nbyte,32);     // шлет очередную порцию данных в FIFO
            nbyte -= 32;
            }


        rfm_22_wait_event(0x20);                                // ждет пока FIFO опустеет окончательно

        rfm_22_write_burst(0x7f,buffer+npack-nbyte,nbyte);      // шлет оставшиеся байты в FIFO
        }

    rfm_22_wait_event(0x04);                                    // ждет пока пакет дойдет окончательно

	return;
	}





/* Отправляет пакет, предварительно проверяя уровень сигнала. startn - указатель на передаваемые данные, lenght - длина пакета.
	Если вызвать с параметрами 0,0 - передает пустой пакет (команду). Задержка, связанная с ожиданием передачи пакета учтена. */

void rfm_22_transmit_small_packet(uint8_t startn, unsigned char lenght) {

//	rfm_22_write(0x07,0x05);							// вкл приемник

//	rfm_22_write(0x08,0x01);							// стирает fifo
//	rfm_22_write(0x08,0x00);

	if (lenght)                                         // если размер пакета не 0
        {
		rfm_22_write(0x3e,lenght);						// размер пакета данных
		rfm_22_write_burst(0x7f,buffer+startn,lenght);	// отправляет пакет в fifo
		}
	else if (startn)
        {	 								// если размер пакета 0
		rfm_22_write(0x3e,0x01);						//костыль из за долбанутых настроек порта RFM
		rfm_22_write(0x7f,0xee);
		}

//	_delay_us(220);

//	while(!(RFM_22_GPIO0_PIN & RFM_22_GPIO0_EN));		// проверяет эфир

	rfm_22_write(0x07,0x09);							// вкл режим передатчика

//	delay_timer(lenght+20);								// задержка, чтобы при передаче пакета не беспокоить RFM

	return;
	}




/* Специальная функция для приема пакета в 256 байт. 256й байт считывается из заголовка. */

void rfm_22_receive_packet_256(void) {

    rfm_22_receive_packet();

    buffer[255] = rfm_22_read(0x47);
    ++npack;

	return;
    }




/* Принимает пакет длиной от 1 до 255 байт. Пакет считывается в буфер, размер пакета записывается в глобальную переменную npack.
    Также проверяет контрольную сумму и корректирует байт состояния пакета. */

void rfm_22_receive_packet(void) {

    uint8_t nbyte, interrupt=0;

    rfm_22_write(0x7e,0x36);

    rfm_22_read(0x03);                                                      // обнуляет прерывания RFM

    nbyte = rfm_22_read(0x4b);						// считывает длину пакета
    npack = nbyte;

    while (nbyte>64) {                                                      // пока количество непринятых байт больше 64

        rfm_22_wait_event(0x10);                                            // ждет пока FIFO не заполнится

        rfm_22_read_burst(0x7f, buffer+npack-nbyte, 32);	                // считывает данные
        nbyte -= 32;                                                        // коректирует количество оставшихся байт
        }

    interrupt = rfm_22_wait_event(0x02);                                    // ждет пока дойдет весь пакет

    rfm_22_read_burst(0x7f, buffer+npack-nbyte, nbyte);                    // считывает последние байты

    if ((interrupt & 0x01) | (interrupt == 0)) packet_condition = 0x01;     // проверяет контрольную сумму
    else                                       packet_condition = 0xff;

	return;
	}




/* Считывает данные пакета */

uint8_t rfm_22_read_small_packet(uint8_t startn) {

	uint8_t lenght;

		lenght = rfm_22_read(0x4b);							//считывает длину пакета

		rfm_22_read_burst(0x7f, buffer+startn, lenght);	    //считывает данные

		rfm_22_write(0x07,0x05);

	rfm_22_write(0x08,0x02);								//стирает fifo
	rfm_22_write(0x08,0x00);

	return lenght;
	}



