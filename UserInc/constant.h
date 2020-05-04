/*
 * constant.h
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv2, free software
 */

#ifndef CONSTANT_H_
#define CONSTANT_H_


/******common define*********/
#define DELAY_BASE_10MIL_TIME			10

#define DELAY_BASE_SEC_TIME				1000

/**WIFI define */
#define WIFI_RX_BUF_SIZE				1024
#define WIFI_TX_BUF_SIZE				(WIFI_RX_BUF_SIZE * 2)
#define WIFI_UART						USART3

#define EVENTBIT_WIFI_UART_REC			(1<<0)
#define EVENTBIT_WIFI_UART_TC			(1<<0)

/**********DEBUG UART2 define************/
#define DEBUG_RX_BUF_SIZE				100
#define DEBUG_TX_BUF_SIZE				(1024)
#define DEBUG_UART						USART2
#define EVENTBIT_DEBUG_UART_REC			(1<<0)
#define EVENTBIT_DEBUG_UART_TC			(1<<1)


#endif /* CONSTANT_H_ */
