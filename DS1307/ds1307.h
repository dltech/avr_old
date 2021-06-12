#ifndef DS1307_H_INCLUDED
#define DS1307_H_INCLUDED

/** Библиотека для часов реального времени DS1307 с программным интерфейсом I2C
	@author Михаил Белкин
*/

/**
Формат записи времени, передаваемый через функции ds_set_time() и ds_read_time()

{секунды | минуты | часы | день недели | число | месяц | год}
**/

/* режим работы часов (12 или 24 часовой) */

//#define H12
#define H24

/* конфигурация лап программного I2C */

#define I2C_BAUDRATE    100000  // скорость передачи данных бод/с

#define DS_I2C_DDR  DDRD
#define DS_I2C_PORT PORTD
#define DS_I2C_PIN  PIND

#define SDA         4
#define SCL         5


/* остальная емуть */

#define I2C_TIME2   500000./I2C_BAUDRATE
#define I2C_TIME4   250000./I2C_BAUDRATE

#define DS1307_ADDRESS_W    0xd0
#define DS1307_ADDRESS_R    0xd1

#define DS1307_NSRAM        56
#define DS1307_CHECK_WORD          0xed


/* Низкоуровневые функции, относящиеся к I2C */

void ds_i2c_init(void);
void ds_i2c_start(void);
void ds_i2c_stop(void);
uint8_t ds_i2c_write_byte(uint8_t byte);
uint8_t ds_i2c_read_byte(void);


/* Функции чтения и записи данных из DS1307 */

uint8_t ds_write_byte(uint8_t address, uint8_t data);
uint8_t ds_write(uint8_t address, uint8_t *data, uint8_t ndata);
uint8_t ds_read_byte(uint8_t address);
uint8_t ds_read(uint8_t address, uint8_t *data, uint8_t ndata);

/* Высокоуровневые функции (в дальнейшем только они и будут нужны). */

uint8_t ds_init(uint8_t *data, uint8_t lenght);
uint8_t ds_set_time(uint8_t *time);
uint8_t ds_read_time(uint8_t *time);




#endif // DS1307_H_INCLUDED
