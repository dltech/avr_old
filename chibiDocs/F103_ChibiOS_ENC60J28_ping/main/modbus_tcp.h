#ifndef MODBUS_TCP_H_INCLUDED
#define MODBUS_TCP_H_INCLUDED

/** @file rfm.h @brief Заголовочный файл */ 

#include <inttypes.h>

/*     Настройки Modbus RTU шлюза				 */
#define WINDOW_SIZE			500




#define TRANSACTION_IDH	0
#define TRANSACTION_IDL	1
#define PROTOCOL_IDH		2
#define	PROTOCOL_IDL		3
#define LENGHT_H				4
#define	LENGHT_L				5
#define	UNIT_ID					6

#define	PDU_POINTER			6

#define MODBUS_PROTOCOL_ID	0

enum {
	MODBUS_SERIAL_ERROR = 1,
	MODBUS_READY,
	MODBUS_REQUEST_GET,
	MODBUS_REQUEST_SENT,
	MODBUS_RESPONSE_GET,
	MODBUS_RESPONSE_SENT,
	MODBUS_TIMEOUT_EXPIRED
};


typedef struct modbusSerialStateStruct {
	uint8_t state;
	uint16_t packLen;
} modbusSerialState_t;


struct modbus_tcp_states {
  uint8_t state;
  uint16_t count;
  char *dataptr;
  char *script;
};


/**** uip related functions ****/

void modbusInit(void);
void modbus_tcp_gateway(void);


typedef struct modbus_tcp_states uip_tcp_appstate_t;

#define UIP_APPCALL modbus_tcp_gateway
#define UIP_APPSTATE_SIZE sizeof(struct modbus_tcp_states)


#endif // MODBUS_TCP_H_INCLUDED
