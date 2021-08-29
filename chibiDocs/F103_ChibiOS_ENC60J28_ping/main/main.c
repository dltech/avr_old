#include <string.h>
#include <stdio.h>

#include "main.h"
#include "stm32f10x.h"

#include "ch.h"
#include "hal.h"
#include "enc.h"
#include "rfm.h"
#include "uip.h"
#include "uip_arp.h"
#include "evtimer.h"
#include "modbus_tcp.h"
#include "util.h"


/*****************************************************
	Global variables
******************************************************/

// Указатели на поток
Thread *uartTp = NULL;
Thread *tp2 = NULL;

// Буферы
static uint8_t rxbuf[16];
//static uint8_t txbuf[16];

// RFM69 variables
extern uint8_t packet_buffer[];
extern uint8_t rfm69_condition;

uint8_t *ptr;

//static VirtualTimer vt1;

extern modbusSerialState_t modbusSerialState;
extern uint8_t modbusSerialBuffer[256];

static void rxend(UARTDriver *uartp);


/*****************************************************
	Peripherial configuration structures 
******************************************************/
// Структура конфигурации UART-драйвера
static UARTConfig uart_cfg_1 = {
  NULL,
  NULL,
  rxend,
  NULL,
  NULL,
  230400,
  0,
  USART_CR2_LINEN,
  0
};

// Struct connect to ENC60J28
// SPI1 on APB2 (72MHz)
// ENC28J60 max freq = 25MHz
// SPI_CR1_BR_0 // - 72MHz/4 = 18MHz - OK!
struct enc encST = {
	&SPID1,
	{
		NULL,
		ENC_CS_PORT,
		ENC_CS_PIN,
		SPI_CR1_BR_0					// fclk/4
	},
	(uint8_t) 0,
	(uint16_t) 0,
	{
	ETHADDR0, ETHADDR1, ETHADDR2, ETHADDR3, ETHADDR4, ETHADDR5
	}
};

// SPI2 on APB1 (36MHz)
// RFM69 max freq = 10MHz
// SPI_CR1_BR_0 // - 36MHz/4 = 9MHz - OK!
struct rfmconf RfmSpiconfig = {
	&SPID2,
	{
		NULL,
		RFM_NSS_PORT,
		RFM_NSS_PIN,
		SPI_CR1_BR_0
	}
};

static const struct uip_eth_addr macaddr = {
	{ETHADDR0, ETHADDR1, ETHADDR2, ETHADDR3, ETHADDR4, ETHADDR5}
};

/*****************************************************
	Mutexes
******************************************************/
// Mutex for SPI protection
static Mutex mtx;

Mutex uartMtx;


/*****************************************************
	Events
******************************************************/

static EVENTSOURCE_DECL(enc_int_es);
static EVENTSOURCE_DECL(rfm_int_event);
static EVENTSOURCE_DECL(button_int_event);
			 EVENTSOURCE_DECL(send_req_event);

EvTimer per_evt;
EvTimer arp_evt;
//EvTimer poll_evt;

static void enc_int(EXTDriver *drv, expchannel_t c)
{
	(void)drv;
	(void)c;
	chEvtBroadcast(&enc_int_es);
}

static void rfm_int(EXTDriver *drv, expchannel_t c)
{
	(void)drv;
	(void)c;
	chEvtBroadcast(&rfm_int_event);
}

static void button_int(EXTDriver *drv, expchannel_t c)
{
	(void)drv;
	(void)c;
	chEvtBroadcast(&button_int_event);
}

static const EXTConfig extcfg = {
	{
		{EXT_CH_MODE_FALLING_EDGE | EXT_MODE_GPIOA | EXT_CH_MODE_AUTOSTART, button_int},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_FALLING_EDGE | EXT_MODE_GPIOB | EXT_CH_MODE_AUTOSTART, enc_int},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOB | EXT_CH_MODE_AUTOSTART, rfm_int},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL},
		{EXT_CH_MODE_DISABLED, NULL}
	}
};

#define EVID_NET_INT			0x01
#define EVID_NET_PERIODIC	0x02
#define EVID_NET_ARP			0x04
#define EVID_NET_POLL			0x08

#define EVID_RFM_DIO0			0x10
#define EVID_BUTTON				0x20

#define EVID_SEND_REQ			0x02

/*****************************************************
	Auxiliary functions
******************************************************/


static void network_send(void)
{
	if (uip_len <= UIP_LLH_LEN + 40)
	{
		enc_packet_send(&encST, uip_len, uip_buf, 0, NULL);
	}
	else
	{
		enc_packet_send(&encST, 54, uip_buf, uip_len - UIP_LLH_LEN - 40, uip_appdata);
	}
}

static void network_error_signal(void)
{
	senddbgmsg("network_error\n");
}

// -----------------------------------------------------------
// FIX
// -----------------------------------------------------------
static void enc_fix_error(void)
{
	uint8_t estat_enc = enc_read_REG(&encST, ESTAT);
	uint8_t eir_enc = enc_read_REG(&encST, EIR);
	uint8_t eie_enc = enc_read_REG(&encST, EIE);
	uint8_t econ1_enc = enc_read_REG(&encST, ECON1); 

//	sprintf(message, "ESTAT-%02x\nEIR-%02x\nEIE-%02x\nECON1-%02x\n", estat_enc, eir_enc, eie_enc, econ1_enc);
//	uartStartSend(&UARTD2, 32, message);

/*	
	uint8_t estat_enc = 0x00;//enc_read_REG(&encST, ESTAT);
	uint8_t eir_enc = 0x00;//enc_read_REG(&encST, EIR);
	uint8_t eie_enc = 0x00;//enc_read_REG(&encST, EIE);
	uint8_t econ1_enc = 0x04;//enc_read_REG(&encST, ECON1); 
//	uint8_t econ2_enc = enc_read_REG(&encST, ECON2);
*/
	if (eir_enc & ENC_RXERIF)
	{
		network_error_signal();
		enc_bit_clr(&encST, EIR, ENC_RXERIF);
 	}
	
	if (estat_enc & ESTAT_LATECOL)
	{
		network_error_signal();
		enc_bit_clr(&encST, ESTAT, ESTAT_LATECOL);
	}
	
	if (estat_enc & ESTAT_BUFER)
	{
		network_error_signal();
		enc_init(&encST);
	}
	
	if (!(econ1_enc & ECON1_RXEN))
	{
		network_error_signal();
		enc_bit_set(&encST, ECON1, ECON1_RXEN);
	}
	
	if (econ1_enc & ECON1_CSUMEN)
	{
		network_error_signal();
		enc_bit_clr(&encST, ECON1, ECON1_CSUMEN);
	}
	
	if (eie_enc != (EIE_INTIE | EIE_PKTIE | EIE_RXERIE))
	{
		network_error_signal();
		enc_init(&encST);
	}
}

// Эта функция вызывается когда буфер приёмника заполнен
static void rxend(UARTDriver *uartp)
{
	chSysLock();
		chSchWakeupS(uartTp, RDY_OK);
	chSysUnlock(); 
}


/*****************************************************
	Threads
******************************************************/

//// -----------------------------------------------------------
//// UART thread
//// -----------------------------------------------------------
static WORKING_AREA(uart_wa, 512);
static msg_t uart_thread(void *p)
{
	chRegSetThreadName("UART thread");
	uartTp = chThdSelf();
		
  while (1)
	{
		// Sleep thread
		chSysLock();
			chSchGoSleepS(THD_STATE_SUSPENDED);
		chSysUnlock();

		senddbgmsg("echo_%0x\n",rxbuf[0]);

		// Разрешение приёма (если не разрешить, rxend перестанет вызываться и начнёт работать rxchar)
		uartStartReceive(&UARTD2, 1, &rxbuf);
  }
	return 0;
}

//// -----------------------------------------------------------
//// NET
//// -----------------------------------------------------------
static WORKING_AREA(network_wa, 2048);
static msg_t network_thread(void *arg)
{
	uip_ipaddr_t ip = {0 , 0};

	EventListener per_el;
	EventListener arp_el;
	EventListener poll_el;
	EventListener int_el;
	
	eventmask_t event;
	int i;
	int pollCnt = 0;
	uint8_t flags;

	(void)arg;
	
	tp2 = chThdSelf();	
	
	// After chThdExit in main
	chSysLock();

		chEvtRegister(&enc_int_es, &int_el, EVID_NET_INT);
		chEvtRegister(&(per_evt.et_es), &per_el, EVID_NET_PERIODIC);
		chEvtRegister(&(arp_evt.et_es), &arp_el, EVID_NET_ARP);
//		chEvtRegister(&(poll_evt.et_es), &poll_el, EVID_NET_POLL);

		// инииализируются события, которые будут запускаться с определенной периодичностью

		
		// запускается SPI и инициализируется контроллер
		chMtxLock(&mtx);
		enc_init(&encST);
		chMtxUnlock();
		
		// выставляются адреса
		uip_init();
		uip_setethaddr(macaddr);
		//-----------------------
		uip_ipaddr(ip, 192,168,127,111);
	  uip_sethostaddr(ip);
	  uip_ipaddr(ip, 192,168,127,100);
	  uip_setdraddr(ip);
	  uip_ipaddr(ip, 255,255,255,0);
	  uip_setnetmask(ip);
		//-----------------------
		modbusInit();
		
		evtStart(&per_evt);
		evtStart(&arp_evt);
//		evtStart(&poll_evt);

	chSysUnlock();

	while (1)
	{

		if (enc_read_REG(&encST, EIR) & EIR_PKTIF)		// если есть пакет тогда выставляется событие (на случай если прерыания глючат в enc)
		{
			event = EVENT_MASK(EVID_NET_INT);
		}
			
		if (!palReadPad(GPIOB, GPIOB_ENC_INT))				// переспрашиваем о наличии прерывания еще раз (на всякий случай)
		{
			event = EVENT_MASK(EVID_NET_INT);
		}
		else
		{
			event = chEvtWaitOne(EVENT_MASK(EVID_NET_INT) | EVENT_MASK(EVID_NET_PERIODIC) | EVENT_MASK(EVID_NET_ARP) );
//													 EVENT_MASK(EVID_NET_POLL));		// ожидание событий
		}

		chMtxLock(&mtx);
		chSysLock();

			if (event == EVENT_MASK(EVID_NET_INT))
			{
				/* The interrupt line is low for as long as we have packets pending */

				flags = enc_int_flags(&encST);		// читаются флаги

				if (flags & ENC_PKTIF)						// если пакет пришел
				{
					while ((uip_len = enc_packet_receive(&encST, UIP_BUFSIZE, uip_buf)) != 0)
					{
						switch(ntohs((((struct uip_eth_hdr *)&uip_buf[0])->type)))	// тип пришедшего пакета
						{
						case UIP_ETHTYPE_IP:																				// если IP
							uip_arp_ipin();
							uip_input();																							// обработать
							if (uip_len > 0)
							{
								uip_arp_out();																					// отправить пакет ARP запрос (чтобы узанть mac адрес узла сети, IP которого указан в отправляемом пакете)
								network_send();																					// отправить пакет или ARP запрос
							}
							break;
						case UIP_ETHTYPE_ARP:																				// если ARP
							uip_arp_arpin();																					// обработать принятый ARP пакет
							if (uip_len > 0)
							{
								network_send();																					// отправить ARP пакет (если есть)
							}
							break;
						default: /* This usually happens on overflows, hackish but works */
							//enc_init(&encST);
							break;
						}
					}
				}
				
				//enc_fix_error();
				
				//enc_int_clear(&encST, ENC_ALLIF);
				/* TODO: This patches the race condition, fix... */
				//chThdSleepMicroseconds(100);
			}
			else if (event == EVENT_MASK(EVID_NET_PERIODIC))									// периодическое сетевое событие
			{
				for (i = 0; i < UIP_CONNS; i++)
				{
					uip_periodic(i);
					if (uip_len > 0)
					{
						uip_arp_out();
						network_send();
					}
				}
			}
// 			else if(event == EVENT_MASK(EVID_NET_POLL))
// 			{
// 				uip_poll_conn(&uip_conns[pollCnt]);
// 				if(++pollCnt == UIP_CONNS) pollCnt = 0;
// 			}
			else
			{
				uip_arp_timer();
			}
			
			enc_fix_error();
			
			// If more unread packets
			if (enc_read_REG(&encST, EPKTCNT) != 0)
			{
				event = EVENT_MASK(EVID_NET_INT);
			}
			
			if (enc_read_REG(&encST, EIR) & EIR_PKTIF)
			{
				event = EVENT_MASK(EVID_NET_INT);
			}

		chMtxUnlock();
		chSysUnlock();

	}
	return 0;
}

// -----------------------------------------------------------
// RFM
// -----------------------------------------------------------
static WORKING_AREA(waThreadRFM, 512);
static msg_t ThreadRFM(void *arg)
{
	EventListener int_rfm_D0;
	eventmask_t event_rfm;
	chEvtRegister(&rfm_int_event, &int_rfm_D0, EVID_RFM_DIO0);
	
	spiStart(&SPID2, &RfmSpiconfig.config);	// Настройка SPI
	spiAcquireBus(&SPID2);            		// Получить эксклюзивный доступ к шине SPI
	rfm_init(&RfmSpiconfig);
	spiReleaseBus(&SPID2);              	// Отменить эксклюзивный доступ к шине SPI
	spiStop(&SPID2);											// Отключить SPI

	while (1)
	{

		event_rfm = chEvtWaitOne(EVENT_MASK(EVID_RFM_DIO0));
		
		if (event_rfm == EVENT_MASK(EVID_RFM_DIO0))
		{
			spiStart(&SPID2, &RfmSpiconfig.config);	// Настройка SPI
			spiAcquireBus(&SPID2);            		// Получить эксклюзивный доступ к шине SPI
			
			//rfm_stby (&RFMconfig);
			rfm_receive_start(&RfmSpiconfig);
			
			spiReleaseBus(&SPID2);              	// Отменить эксклюзивный доступ к шине SPI
			spiStop(&SPID2);											// Отключить SPI
		}
	}
	return 0;
}

// -----------------------------------------------------------
// BLINK, RESET SPI & ENC on button
// -----------------------------------------------------------
static WORKING_AREA(waThreadBlink, 1024);
static msg_t ThreadBlink(void *arg)
{
	static uint8_t state_prev;

	while (TRUE)
	{
		chSysLock();
			palTogglePad(GPIOA, GPIOA_LED_GREEN);
		chSysUnlock();

		chMtxLock(&mtx);
			enc_fix_error();
		chMtxUnlock();

		// chSysLock();
		// 	if( modbusSerialState.state != state_prev)
		// 	{
		// 		senddbgmsg("state = %d\n", modbusSerialState.state);
		// 	}
		// 	state_prev = modbusSerialState.state;
		// chSysUnlock();

		chThdSleepMilliseconds(1000);
	}
	return 0;
}

// -----------------------------------------------------------
// thread for educational purposes
// -----------------------------------------------------------
static WORKING_AREA(waThreadButton, 512);
static msg_t ThreadButton(void *arg)
{
	EventListener button_int_el;

	chEvtRegister(&button_int_event, &button_int_el, EVID_BUTTON);

	while(1)
	{
		chEvtWaitOne(EVENT_MASK(EVID_BUTTON));
		senddbgmsg("button pressed\n");
	}
	return 0;
}


// -----------------------------------------------------------
// modbus serial line thread
// -----------------------------------------------------------
static WORKING_AREA(waThreadModbusSerial, 1024);
static msg_t ThreadModbusSerial(void *arg)
{
	char number[10];
	int i;
	EventListener send_req_el;

	chEvtRegister(&send_req_event, &send_req_el, EVID_SEND_REQ);

	while(1)
	{
		chEvtWaitOne(EVENT_MASK(EVID_SEND_REQ));

		senddbgmsg("pack get %01x\n", modbusSerialState.state);
		if(modbusSerialState.state == MODBUS_REQUEST_GET)
			modbusSerialState.state = MODBUS_RESPONSE_GET;



//		sprintf(message, "pack get, pid=%02x%02x,len=%02x,uid=%02x\n",modbusSerialBuffer[PROTOCOL_IDH],modbusSerialBuffer[PROTOCOL_IDL],modbusSerialBuffer[LENGHT_H],modbusSerialBuffer[UNIT_ID]);
/*
		for(i=0 ; i<modbusSerialState.packLen ; ++i)
		{
			sprintf(number, "%02x", modbusSerialBuffer[i]);
			strcat(message, number);
		}
		sprintf(number, "pack\n");
		strcat(message, number);
*/
//		uartStartSend(&UARTD2, strlen(message), message);
	}
	return 0;
}


// -----------------------------------------------------------
// MAIN
// -----------------------------------------------------------
int main(void)
{
	uint32_t temp;

	chEvtInit(&rfm_int_event);
	chEvtInit(&enc_int_es);
	chEvtInit(&send_req_event);

	evtInit(&per_evt, MS2ST(1000));
//	evtInit(&poll_evt, MS2ST(300));
	evtInit(&arp_evt, S2ST(10));

	chMtxInit(&mtx);
	chMtxInit(&uartMtx);
	chSysInit();
	
	halInit();
	spiStart(&SPID1, &encST.config);
	extStart(&EXTD1, &extcfg);
	uartStart(&UARTD2, &uart_cfg_1);

	chMtxLock(&uartMtx);
  	uartStartSend(&UARTD2, 13, "Starting...\r\n");
  chMtxUnlock();
	
	// Disable JTAG
	temp = AFIO->MAPR;
	temp |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	AFIO->MAPR = temp;
	
	// All led = off
	palClearPort(GPIOA, 0xFFFFFFFF);
	palClearPort(GPIOB, 0xFFFFFFFF);
	
	//UART
	chThdCreateStatic(uart_wa, sizeof(uart_wa), NORMALPRIO, uart_thread, NULL);
	// NET
	chThdCreateStatic(network_wa, sizeof(network_wa), NORMALPRIO, network_thread, NULL);
	// BLINK
	chThdCreateStatic(waThreadBlink, sizeof(waThreadBlink), NORMALPRIO, ThreadBlink, NULL);
	// RFM
	chThdCreateStatic(waThreadRFM, sizeof(waThreadRFM), NORMALPRIO, ThreadRFM, NULL);
	// Button
	chThdCreateStatic(waThreadButton, sizeof(waThreadButton), NORMALPRIO, ThreadButton, NULL);
	// Modbus serial
	chThdCreateStatic(waThreadModbusSerial, sizeof(waThreadModbusSerial), NORMALPRIO, ThreadModbusSerial, NULL);

	// Enable receive UART2
	uartStartReceive(&UARTD2, 1, &rxbuf);

	chThdExit(0);

	while (1)
	{
		chThdSleepMilliseconds(5000);
  }
	
	return (1);
}