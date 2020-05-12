/*
 * wifiUtil.c
 *
 *  Created on: May 7, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */
#include "wifiUtil.h"
#include "usart.h"

extern TaskHandle_t wifiAPTaskHandle;
extern WifiUsartData_S wifiData;
extern EventGroupHandle_t wifiEventHandler;

WIFI_UART_RSP_E wifi_send_uart(char * buffer, char * sucInfo1, char * sucInfo2, char * errInfo, uint32_t len,
		uint32_t waittime, bool flag)
{
	EventBits_t eventValue;
	uint8_t txData[WIFI_TX_BUF_SIZE] = { 0 };
	uint8_t endflag[2] = { 0x0d, 0x0a };
	uint32_t size;

	if (len > 0)
	{
		size = len;
	}
	else
	{
		size = strlen(buffer);
	}


	if ((size > (WIFI_TX_BUF_SIZE - 2)) || (buffer == NULL))
	{
		printf("the send data too long or is null!\n");
		return ERR;
	}
	if (flag)
	{
		sprintf(txData, "%s%c%c", buffer, endflag[0], endflag[1]);
		size = size + 2;
	}
	else
	{
		memcpy(txData, buffer, size);
	}

	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
	wifi_uart_send(txData, size);

	if ((sucInfo1 == NULL) && (sucInfo2 == NULL))
	{
		return SUC;
	}

	eventValue = xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC, pdTRUE, pdFALSE, waittime);
	if ((eventValue & EVENTBIT_WIFI_UART_REC) == 0)
	{
		printf("WIFI UART get response overtime\n");
		return OVERTIME;
	}
	if ((sucInfo1 != NULL) && (strstr(wifiData.netUsartRxBuffer[wifiData.index], sucInfo1)))
	{
		return SUC;
	}
	else if ((sucInfo2 != NULL) && (strstr(wifiData.netUsartRxBuffer[wifiData.index], sucInfo2)))
	{
		return SUC;
	}
	else if ((errInfo != NULL) && strstr(wifiData.netUsartRxBuffer[wifiData.index], errInfo))
	{
		return FAILED;
	}

	printf("WIFI UART get the error response\n");

	return ERR;

}

WIFI_UART_RSP_E wifi_send_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, NULL, errInfo, 0, waittime, true);
}

WIFI_UART_RSP_E wifi_send_cmd_mux(char * buffer, char * sucInfo1, char * sucInfo2, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo1, sucInfo2, errInfo, 0, waittime, true);
}

WIFI_UART_RSP_E wifi_send_data(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, NULL, errInfo, 0, waittime, false);
}

void wifi_send_mqtt_data(char * buffer, char * sucInfo, char * errInfo, uint32_t len, uint32_t waittime)
{
	wifi_send_uart(buffer, sucInfo, NULL, errInfo, len, waittime, false);
}

