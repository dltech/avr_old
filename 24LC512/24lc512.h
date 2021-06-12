#ifndef LC512_H_INCLUDED
#define LC512_H_INCLUDED

/** Библиотека для внешнего eeprom 24LC512 с программным интерфейсом I2C, на случай если не попал в аппаратный
	@author Михаил Белкин
*/


#define SEEP_I2C_BAUDRATE    1000000  // скорость передачи данных бод/с
#define SEEP_TIMEOUT_MS      100     // наибольшее время ожидания следущей записи в eeprom

#define SEEP_I2C_DDR  DDRA
#define SEEP_I2C_PORT PORTA
#define SEEP_I2C_PIN  PINA

#define SEEP_SDA         6
#define SEEP_SCL         7

/* остальная емуть */

#define SEEP_I2C_TIME2   500000./SEEP_I2C_BAUDRATE
#define SEEP_I2C_TIME4   250000./SEEP_I2C_BAUDRATE

#define SEEP_TIMEOUT     SEEP_TIMEOUT_MS/(36000./SEEP_I2C_BAUDRATE)

#define SEEP_ADDRESS_W    0xa0
#define SEEP_ADDRESS_R    0xa1

#define SEEP_MAX_POINTER    0xf9ff



/* Низкоуровневые функции, относящиеся к I2C */

void seep_i2c_init(void);
void seep_i2c_start(void);
void seep_i2c_stop(void);
void seep_i2c_write_ack(void);
uint8_t seep_i2c_read_ack(void);
void seep_i2c_write_byte(uint8_t byte);
uint8_t seep_i2c_read_byte(void);


/* Функции чтения и записи данных из 24LC512 */

uint8_t seep_init(void);
void seep_wait(void);
uint8_t seep_write_byte(uint16_t address, uint8_t data);
uint8_t seep_write(uint16_t address, uint8_t *data, uint8_t ndata);
uint8_t seep_read_byte(uint16_t address);
uint8_t seep_read(uint16_t address, uint8_t *data, uint8_t ndata);


#endif // 24LC512_H_INCLUDED
