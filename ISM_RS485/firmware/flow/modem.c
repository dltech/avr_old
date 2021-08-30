#include <avr/eeprom.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#include <util/delay.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "rfm_22_23.h"
#include "rs485.h"
#include "modem.h"
//#include "route.h"




/* Обработчик пакетов */

void packet_handler(void) {

	rfm_22_read_burst(0x47, header , 3);				// считывает заголовок

    switch (header[1]) {

        case 0x01 : rfm_22_receive_packet();
                    if (packet_condition == 0xff)      // если пакет успешно принят и находится в буфере
                        rs485_transmit_packet();       // передает пакет по RS485
                    else
                        rfm_22_transmit_command(0x06); // иначе просит переслать его снова
                    break;

        case 0x02 : rfm_22_receive_packet_256();
                    if (packet_condition == 0xff)
                        rs485_transmit_packet();
                    else
                        rfm_22_transmit_command(0x06);
                    break;

        case 0x03 : if (packet_condition == 0x01) {
                        rfm_22_receive_packet();
                        if (packet_condition == 0xff)   // если пакет успешно принят и находится в буфере
                            rs485_transmit_packet();    // передает пакет по RS485
                        }
                    packet_condition = 0x00;
                    break;

        case 0x04 : if (packet_condition == 0x01) {
                        rfm_22_receive_packet_256();
                        if (packet_condition == 0xff)   // если пакет успешно принят и находится в буфере
                            rs485_transmit_packet();    // передает пакет по RS485
                        }
                    packet_condition = 0x00;
                    break;

        case 0x06 : if (packet_condition == 0x10) {
                        if (npack == 256) {
                            rfm_22_write(0x3b,0x04);
                            rfm_22_transmit_packet_256();
                            }
                        else {
                            rfm_22_write(0x3b,0x03);
                            rfm_22_transmit_packet();
                            }
                        packet_condition = 0x00;        // обнуляет состояние пакета для исключения еще одной пересылки
                        }
                    break;
        }

    }
