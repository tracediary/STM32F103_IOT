/*
 * constant.h
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#ifndef CONSTANT_H_
#define CONSTANT_H_


/******common define*********/
#define DELAY_BASE_10MIL_TIME			10

#define DELAY_BASE_SEC_TIME				1000

/**WIFI define */
#define DEFAULT_SSID					"siweiwuhen"
#define DEFAULT_PWD						"763505660cqu"

#define WIFI_RX_BUF_SIZE				1024
#define WIFI_TX_BUF_SIZE				(WIFI_RX_BUF_SIZE * 2)
#define WIFI_UART						USART3

#define WIFI_UART_WAITTIME				500

#define EVENTBIT_WIFI_UART_REC			(1<<0)
#define EVENTBIT_WIFI_UART_TC			(1<<1)
#define EVENTBIT_WIFI_CONFIG			(1<<2)
#define EVENTBIT_WIFI_DIS_AP			(1<<3)
#define EVENTBIT_WIFI_CON_ING_AP		(1<<4)
#define EVENTBIT_WIFI_CONNECTED_AP		(1<<5)
#define EVENTBIT_WIFI_DIS_ING_AP		(1<<6)
#define EVENTBIT_WIFI_LINK_0			(1<<7)
#define EVENTBIT_WIFI_LINK_0_READY		(1<<8)
#define EVENTBIT_WIFI_LINK_1			(1<<9)
#define EVENTBIT_WIFI_LINK_1_READY		(1<<10)
#define EVENTBIT_WIFI_LINK_2			(1<<11)
#define EVENTBIT_WIFI_LINK_2_READY		(1<<12)
#define EVENTBIT_WIFI_LINK_3			(1<<13)
#define EVENTBIT_WIFI_LINK_3_READY		(1<<14)
#define EVENTBIT_WIFI_LINK_4			(1<<15)
#define EVENTBIT_WIFI_LINK_4_READY		(1<<16)
#define EVENTBIT_WIFI_MUX				(1<<17)
#define EVENTBIT_WIFI_SINGLE_MODE		(1<<18)


/**********DEBUG UART2 define************/
#define DEBUG_RX_BUF_SIZE				100
#define DEBUG_TX_BUF_SIZE				(1024)
#define DEBUG_UART						USART2
#define EVENTBIT_DEBUG_UART_REC			(1<<0)
#define EVENTBIT_DEBUG_UART_TC			(1<<1)








#endif /* CONSTANT_H_ */
