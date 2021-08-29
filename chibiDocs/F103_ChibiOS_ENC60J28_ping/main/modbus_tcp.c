#include "modbus_tcp.h"
#include "uip.h"
#include "ch.h"
#include "hal.h"
#include "util.h"

#include <string.h>

/*********************************************************
@author Mikhail Belkin
											functions
*********************************************************/

uint8_t modbusSerialBuffer[256];

modbusSerialState_t modbusSerialState;


extern EventSource send_req_event;

/*
struct modbusWindowStruct {
	uint8_t buffer[WINDOW_SIZE];
	const uint8_t *startPtr = buffer;
	const uint8_t *endPtr = buffer + WINDOW_SIZE;
	uint8_t *inPtr = buffer;
	uint8_t *outPtr = buffer;
} modbusWindow;
*/
void modbusInit(void)
{
	uip_listen(HTONS(502));
	modbusSerialState.state = MODBUS_READY;
}

void changeState(int newState)
{
	modbusSerialState.state = newState;
	senddbgmsg("newSt %d", newState);
}


int isValid(uint8_t *packBuffer, uint8_t packLen)
{
/*	if( (packBuffer[PROTOCOL_IDH] == (MODBUS_PROTOCOL_ID  & 0xff) )      &&		// совпадает ли идентификатор протокола
			(packBuffer[PROTOCOL_IDL] == ((MODBUS_PROTOCOL_ID >> 8) & 0xff)) &&
			(packBuffer[LENGHT_H]     == (packLen - 6) )							       &&		// размер пакета совпадает с размером, указанным в заголовке
			(packBuffer[UNIT_ID]      <  248 )													        )	// адрес устройства находится в пределах допустимых значений	
		return 1;
	else
	{

		return 0;
	} */
	if(packLen < UNIT_ID)
	{
		senddbgmsg("len is null\n");
		return FALSE;
	}
	else if( (packBuffer[PROTOCOL_IDH] != (MODBUS_PROTOCOL_ID  & 0xff) )      ||		// совпадает ли идентификатор протокола
					 (packBuffer[PROTOCOL_IDL] != ((MODBUS_PROTOCOL_ID >> 8) & 0xff))    )
	{
		senddbgmsg("prot id is nok\n");
		return FALSE;
	}
	else if ( ((packBuffer[LENGHT_H] << 8) + packBuffer[LENGHT_L]) != (packLen - 6) )	
	{
		senddbgmsg("len is nok\n");
		return FALSE;
	}
	else if(packBuffer[UNIT_ID]      >  248 )
	{
		senddbgmsg("unit id is nok\n");
		return FALSE;
	}	
	else return TRUE;

}
/*
void pushPacket(uint8_t *packBuffer, uint8_t packetLen)
{
	for( i = 0 ; i < packetLen ; ++i )
	{
		if( modbusWindow.inPtr == modbusWindow.endPtr )	
			modbusWindow.inPtr = modbusWindow.startPtr;
		if( modbusWindow.inPtr == modbusWindow.outPtr )
			pullPacket();

		*(modbusWindow.inPtr++) = *(packBuffer++);
	}
}

uint8_t pullPacket(uint8_t *packBuffer)
{
	uint8_t packetLen = modbusWindow.outPtr[LENGHT_L] +
										 (modbusWindow.outPtr[LENGHT_H] << 8) + 6;

	for( i = 0 ; i < packetLen ; ++i )
	{
		if( modbusWindow.outPtr == modbusWindow.endPtr )	
			modbusWindow.outPtr = modbusWindow.startPtr;
		if( modbusWindow.outPtr == modbusWindow.inPtr )
		{
			modbusWindow.outPtr = modbusWindow.startPtr;
			modbusWindow.inPtr  = modbusWindow.startPtr;
		}

		*(packBuffer++) = *(modbusWindow.outPtr++);
	}
	return packetLen; 
}

void nullPacket(void)
{
	uint8_t packetLen = modbusWindow.outPtr[LENGHT_L] +
										 (modbusWindow.outPtr[LENGHT_H] << 8) + 6;

	for( i = 0 ; i < packetLen ; ++i )
	{
		if( modbusWindow.outPtr == modbusWindow.endPtr )	
			modbusWindow.outPtr = modbusWindow.startPtr;
		if( modbusWindow.outPtr == modbusWindow.inPtr )
		{
			modbusWindow.outPtr = modbusWindow.startPtr;
			modbusWindow.inPtr  = modbusWindow.startPtr;
		}

		*(modbusWindow.outPtr++) = 0;
	}
}
*/

void modbus_tcp_gateway(void)
{
	if( uip_newdata() || uip_connected() )
	{
		if( (modbusSerialState.state == MODBUS_READY) || 
				(modbusSerialState.state == MODBUS_TIMEOUT_EXPIRED))
		{		
			if( isValid(uip_appdata, uip_datalen()) )
			{
//				senddbgmsg("%01x\n", modbusSerialState.state);
				memcpy(modbusSerialBuffer, uip_appdata, uip_datalen());
				modbusSerialState.packLen = uip_datalen();

				changeState(MODBUS_REQUEST_GET);
				chEvtBroadcast(&send_req_event);
			}
		}
	}

	if( uip_acked() )
	{
		if( modbusSerialState.state == MODBUS_RESPONSE_SENT )
		{
			changeState(MODBUS_READY);
			senddbgmsg("back2ready\n");
		}
	}

	if( uip_poll() || uip_rexmit() )
	{
		if( modbusSerialState.state == MODBUS_RESPONSE_GET )
		{
			uip_send(modbusSerialBuffer, modbusSerialState.packLen);
			changeState(MODBUS_RESPONSE_SENT);			
			senddbgmsg("pack sent\n");
		}
		else
		{
			senddbgmsg("poll\n");
		}
	}

  if( uip_aborted() || uip_timedout() || uip_closed() ) {
  	senddbgmsg("con abort\n");
  	changeState(MODBUS_READY);
  }

}