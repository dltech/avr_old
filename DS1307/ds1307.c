#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>

#include "ds1307.h"


/** Библиотека для часов реального времени DS1307 с программным интерфейсом I2C, на случай если не попал в аппаратный
	@author Михаил Белкин
*/


/* Служебные функции */

void ds_i2c_init(void)
{
    DS_I2C_DDR |= (1<<SCL) | (1<<SDA);
    DS_I2C_PORT |= (1<<SCL) | (1<<SDA);
}


void ds_i2c_start(void)
{
    _delay_us(I2C_TIME4);
    DS_I2C_PORT &= ~(1<<SDA);
    _delay_us(I2C_TIME4);
    DS_I2C_PORT &= ~(1<<SCL);
    _delay_us(I2C_TIME2);
}


void ds_i2c_stop(void)
{
    DS_I2C_PORT &= ~(1<<SDA);
    _delay_us(I2C_TIME2);
    DS_I2C_PORT |= (1<<SCL);
    _delay_us(I2C_TIME4);
    DS_I2C_PORT |= (1<<SDA);
    _delay_us(I2C_TIME4);
}



/* Отправляет один байт по I2C. Возвращает 0 если подтверждение принято, иначе 1 */

uint8_t ds_i2c_write_byte( uint8_t byte )
{
    uint8_t ack, mask = 0x80;

    while ( mask )
    {
        if (mask & byte)    DS_I2C_PORT |= (1<<SDA);
        else                DS_I2C_PORT &= ~(1<<SDA);
        mask >>= 1;

        _delay_us(I2C_TIME2);
        DS_I2C_PORT |= (1<<SCL);
        _delay_us(I2C_TIME2);
        DS_I2C_PORT &= ~(1<<SCL);
    }

    DS_I2C_DDR &= ~(1<<SDA);
    DS_I2C_PORT |= (1<<SDA);
    _delay_us(I2C_TIME2);

    DS_I2C_PORT |= (1<<SCL);

    _delay_us(I2C_TIME4);
    ack = DS_I2C_PIN & (1<<SDA);
    _delay_us(I2C_TIME4);

    DS_I2C_PORT &= ~(1<<SCL);
    _delay_us(I2C_TIME2);
    DS_I2C_DDR |= (1<<SDA);

    if (ack)
        return 1;
    else
        return 0;
}


/* функция чтения байта по I2C, возвращает прочитаный байт */

uint8_t ds_i2c_read_byte(void)
{
    uint8_t byte = 0, mask = 0x80;

    DS_I2C_DDR &= ~(1<<SDA);

    while ( mask )
    {
        _delay_us(I2C_TIME2);
        DS_I2C_PORT |= (1<<SCL);
        _delay_us(I2C_TIME4);

        if (DS_I2C_PIN & (1<<SDA))
            byte |= mask;
        mask >>= 1;

        _delay_us(I2C_TIME4);
        DS_I2C_PORT &= ~(1<<SCL);
    }

    DS_I2C_DDR |= (1<<SDA);
    DS_I2C_PORT &= ~(1<<SDA);

    _delay_us(I2C_TIME2);
    DS_I2C_PORT |= (1<<SCL);
    _delay_us(I2C_TIME2);
    DS_I2C_PORT &= ~(1<<SCL);
    DS_I2C_PORT |= (1<<SDA);

    return byte;
}


/* Запись нескольких одного байта по I2C. Возвращает значение > 0 если нет связи. */

uint8_t ds_write_byte( uint8_t address, uint8_t data )
{
    uint8_t ack = 0;

    ds_i2c_start();

    ack += ds_i2c_write_byte(DS1307_ADDRESS_W);
    ack += ds_i2c_write_byte(address);
    ack += ds_i2c_write_byte(data);

    ds_i2c_stop();

    return ack;
}


/* Запись нескольких байт по I2C. Возвращает значение > 0 если нет связи. */

uint8_t ds_write( uint8_t address, uint8_t *data, uint8_t ndata )
{
    uint8_t i, ack = 0;

    ds_i2c_start();

    ack += ds_i2c_write_byte(DS1307_ADDRESS_W);
    ack += ds_i2c_write_byte(address);

    for (i=0 ; i<ndata ; ++i)
        ack += ds_i2c_write_byte(data[i]);

    ds_i2c_stop();

    return ack;
}


/* Чтение одного байта по I2C. Возвращает считанное число, если успешно, иначе возвращает 0 */

uint8_t ds_read_byte( uint8_t address )
{
    uint8_t ack = 0, data;

    ds_i2c_start();

    ack += ds_i2c_write_byte(DS1307_ADDRESS_W);
    ack += ds_i2c_write_byte(address);

    ds_i2c_stop();

    ds_i2c_start();

    ack += ds_i2c_write_byte(DS1307_ADDRESS_R);
    data = ds_i2c_read_byte();

    ds_i2c_stop();

    if ( ack > 0 )
        return 0;
    else
        return data;
}


/* Чтение нескольких байт подряд по SPI */

uint8_t ds_read( uint8_t address, uint8_t *data, uint8_t ndata )
{
    uint8_t i, ack = 0;

    ds_i2c_start();

    ack += ds_i2c_write_byte(DS1307_ADDRESS_W);
    ack += ds_i2c_write_byte(address);

    ds_i2c_stop();

    ds_i2c_start();

    ack += ds_i2c_write_byte(DS1307_ADDRESS_R);
    for (i=0 ; i<ndata ; ++i)
        data[i] = ds_i2c_read_byte();

    ds_i2c_stop();

    return ack;
}


/* Инициализация часов. Включает SPI, проверяет, инициализировались ли ранее, если нет - записывает нулевое значение времени и запускает часы. */

uint8_t ds_init (uint8_t *init_data, uint8_t lenght)
{

#ifdef  H24
    uint8_t time[] = {0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x0e};
#endif

#ifdef  H12
    uint8_t time[] = {0x00, 0x00, 0x40, 0x01, 0x01, 0x01, 0x0e};
#endif

    uint8_t check, ack = 0;

    ds_i2c_init();

    check = ds_read_byte(0x08);

    if ( check != DS1307_CHECK_WORD )
    {
        uint8_t buffer[DS1307_NSRAM], i;

        ack = ds_set_time(time);

        buffer[0] = DS1307_CHECK_WORD;

        for (i=1 ; i<=lenght      ; ++i)    buffer[i] = init_data[i-1];
        for (    ; i<DS1307_NSRAM ; ++i)    buffer[i] = 0;

        ack += ds_write(0x08, buffer, DS1307_NSRAM);
    }

    return ack;
}

/* Установка вермени - передается массив из семи байт (секунды, минуты, часы, день недели, дата, месяц, год).
    Возвращает 0 если все прошло успешно - иначе > 0. */

uint8_t ds_set_time (uint8_t *time)
{
    uint8_t data[8], ack, i;

    for (i=0 ; i<7 ; ++i)   data[i] = ((time[i]/10)<<4) + (time[i]%10);

#ifdef  H12
    data[2] = 0x40 | (((time[2]/10)<<4) + (time[2]%10));
    if (time[2] > 12)
        data[2] |= 0x20;
#endif

    data[7] = 0x00;

    ack = ds_write(0x00, data, 8);

    return ack;
}

/* Считывается время - передается массив из семи байт (секунды, минуты, часы, день недели, дата, месяц, год).
    Возвращает 0 если все прошло успешно - иначе > 0. */

uint8_t ds_read_time ( uint8_t *time )
{
    uint8_t ack, i;

    ack = ds_read(0x00, time, 7);

    for (i=0 ; i<7 ; ++i)
        time[i] = ((time[i]>>4)*10) + (time[i]&0x0f);

    return ack;
}

