/*
 * wifi.c
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */
#include "debug.h"
#include "constant.h"
#include "hardware.h"
#include "system.h"

/**FreeRTOS head file**********************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

WifiUsartData_S wifiData = { 0 };
uint8_t wifiTxData[WIFI_TX_BUF_SIZE] = { 0 };

extern EventGroupHandle_t wifiEventHandler;

void wifi_Handle()
{

	wifiData.index = 0;
	xEventGroupSetBits(wifiEventHandler, EVENTBIT_DEBUG_UART_TC);
	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
}

uint8_t wifi_send_cmd(uint8_t *buffer, char * sucInfo, char * errInfo)
{
	uint8_t cmd;
	EventBits_t eventValue;
	uint32_t size = strlen(buffer);

	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
	wifi_uart_send(buffer, size);

	eventValue = xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC, pdTRUE, pdFALSE, WIFI_UART_DELAY);
	if ((eventValue & EVENTBIT_WIFI_UART_REC) == 0)
	{
		//TODO 接收数据失败，打印日志
	}
	//TODO 判断成功失败


	return 0;

}
