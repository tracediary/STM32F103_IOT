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
extern EventGroupHandle_t wifiEventHandler;

WIFI_UART_RSP_E wifi_send_uart(char * buffer, char * sucInfo1, char * sucInfo2, char * errInfo,
		uint32_t len, uint32_t waittime, bool flag)
{
	EventBits_t eventValue;
	uint8_t txData[WIFI_NORMAL_BUF_SIZE] = { 0 };
	uint8_t *txBuf = NULL;
	uint8_t endflag[2] = { 0x0d, 0x0a };
	uint32_t size;
//	uint32_t wait_event_bit = 0;

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
		Error("the send data too long or is null!\n");
		return ERR;
	}

	if (size < (WIFI_NORMAL_BUF_SIZE - 2))
	{
		txBuf = txData;
	}
	else
	{
		txBuf = pvPortMalloc(size + 2);
		if (txBuf == NULL)
		{
			Error("%s apply ram send to wifi failed!\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));
			return ERR;
		}
	}

	if (flag)
	{
		sprintf(txBuf, "%s%c%c", buffer, endflag[0], endflag[1]);
		size = size + 2;
	}
	else
	{
		memcpy(txBuf, buffer, size);
	}

#if 0
	if (socket == WIFI_CMD_FLAG)
	{
		wait_event_bit = EVENTBIT_WIFI_UART_REC;
	}
	else
	{
		wait_event_bit = wifi_link_ready[socket];
	}
#endif

	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
	wifi_uart_send(txBuf, size);

	if ((sucInfo1 == NULL) && (sucInfo2 == NULL))
	{
		return SUC;
	}

	eventValue = xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC, pdTRUE, pdFALSE, waittime);

	if (txBuf != txData)
	{
		vPortFree(txBuf);
		txBuf = NULL;
	}

	if ((eventValue & EVENTBIT_WIFI_UART_REC) == 0)
	{
		printf("%s WIFI UART get response overtime.\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));
		return OVERTIME;
	}
	if ((sucInfo1 != NULL) && (strstr(wifi_uart_curdata_addr, sucInfo1)))
	{
		return SUC;
	}
	else if ((sucInfo2 != NULL) && (strstr(wifi_uart_curdata_addr, sucInfo2)))
	{
		return SUC;
	}
	else if ((errInfo != NULL) && strstr(wifi_uart_curdata_addr, errInfo))
	{
		return FAILED;
	}
	else if (strstr(wifi_uart_curdata_addr, "busy s...") || strstr(wifi_uart_curdata_addr, "busy p..."))
	{
		return BUSY;
	}


	printf("WIFI UART get the error response\n");

	return ERR;

}

#if 0
WIFI_UART_RSP_E wifi_send_link_cmd(uint8_t socket, char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(socket, buffer, sucInfo, NULL, errInfo, 0, waittime, true);
}
#endif
WIFI_UART_RSP_E wifi_send_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, NULL, errInfo, 0, waittime, true);
}

WIFI_UART_RSP_E wifi_send_cmd_mux(char * buffer, char * sucInfo1, char * sucInfo2, char * errInfo,
		uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo1, sucInfo2, errInfo, 0, waittime, true);
}

WIFI_UART_RSP_E wifi_send_data(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, NULL, errInfo, 0, waittime, false);
}

WIFI_UART_RSP_E wifi_send_ascii_data_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t len,
		uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, NULL, errInfo, len, waittime, false);
}

WIFI_STATUS_E get_wifi_status()
{
	uint8_t repeatTime = 0;
	const uint8_t maxTime = 10;
	WIFI_UART_RSP_E wifi_status_rsp;


	GET_STATUS:


	wifi_status_rsp = wifi_send_cmd("AT+CIPSTATUS", "STATUS:", NULL, (WIFI_UART_WAITTIME * 4));
	if (SUC == wifi_status_rsp)
	{
		if (strstr(wifi_uart_curdata_addr, "STATUS:2"))
		{
			return CON_AP;
		}
		else if (strstr(wifi_uart_curdata_addr, "STATUS:3"))
		{
			return CON_TCP_UDP;
		}
		else if (strstr(wifi_uart_curdata_addr, "STATUS:4"))
		{
			return DIS_CON_TCP_UDP;
		}
		else if (strstr(wifi_uart_curdata_addr, "STATUS:5"))
		{
			return DIS_CON_AP;
		}
	}
	else if (BUSY == wifi_status_rsp && repeatTime++ < maxTime)
	{
		vTaskDelay(DELAY_BASE_SEC_TIME * 4);
		goto GET_STATUS;
	}
	else
	{
		return DIS_CON_AP;
	}

}







