/*
 * system.h
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#ifndef USERINC_SYSTEM_H_
#define USERINC_SYSTEM_H_

#include "constant.h"
#include "debug.h"
#include "stdio.h"
#include "string.h"
#include <stdbool.h>

/***FreeRTOS var define**************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

typedef struct _WIFI_USART_DATA
{
	uint8_t index;
	uint8_t netUsartRxBuffer[2][WIFI_RX_BUF_SIZE];

} WifiUsartData_S;


typedef struct _WIFI_AP_SECRET
{
	char ssid[32];
	char pwd[32];

} WIFI_AP_SECRET_S;



typedef enum
{
	SUC = 1, FAILED, OVERTIME, ERR
} WIFI_UART_RSP_E;


typedef enum
{
	CONFIG = 1, DIS_CONNECT, CONNECTING, CONNECTED, DIS_CONNECTING
} WIFI_AP_STATUS_E;




typedef enum
{
	STA = 1, AP, STA_AP
} WIFI_NET_E;





//TODO 待删除定义

typedef struct
{
	int humidity;        //实际湿度
	int temperature;     //实际温度
} DHT11_Data_TypeDef;







#endif /* USERINC_SYSTEM_H_ */
