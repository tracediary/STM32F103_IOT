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

#define DELAY_BASE_SEC_TIME				(DELAY_BASE_10MIL_TIME * 100)

#define DELAY_BASE_MIN_TIME				(DELAY_BASE_SEC_TIME * 60)

#define TIME_ZONE						8

/**WIFI define start*****************************************************/
#define DEFAULT_SSID					"siweiwuhen"
#define DEFAULT_PWD						"763505660cqu"

#define WIFI_RX_BUF_SIZE				500
#define WIFI_TX_BUF_SIZE				WIFI_RX_BUF_SIZE
#define WIFI_NORMAL_BUF_SIZE			50
#define WIFI_UART						USART3

//除了MQTT，其他名字都可以更改
#define WIFI_CMD_FLAG					99
#define WIFI_MAX_LINK					5

#define WIFI_MQTT_LINK_ID				0
#define WIFI_SNTP_LINK_ID				1
#define WIFI_TCP2_LINK_ID				2
#define WIFI_UDP1_LINK_ID				3
#define WIFI_UDP2_LINK_ID				4

#define WIFI_UART_WAITTIME				(DELAY_BASE_10MIL_TIME * 50)

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




/**WIFI define end*****************************************************/

/**********DEBUG UART2 define start************************************/
#define DEBUG_RX_BUF_SIZE				100
#define DEBUG_TX_BUF_SIZE				(1024)
#define DEBUG_UART						USART2
#define EVENTBIT_DEBUG_UART_REC			(1<<0)
#define EVENTBIT_DEBUG_UART_TC			(1<<1)
/**********DEBUG UART2 define end**************************************/

/**********system define start*****************************************/

#define EVENTBIT_SYS_SYNC_TIME			(1<<0)


/**********system define end*******************************************/
#endif /* CONSTANT_H_ */
