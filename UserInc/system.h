/*
 * system.h
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv2, free software
 */

#ifndef USERINC_SYSTEM_H_
#define USERINC_SYSTEM_H_


typedef struct _WIFI_USART_DATA
{
	uint8_t index;
	char netUsartRxBuffer[2][NET_USART_REC_LEN];

} WifiUsartData_S;


#endif /* USERINC_SYSTEM_H_ */
