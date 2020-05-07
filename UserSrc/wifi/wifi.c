/*
 * wifi.c
 *
 *  Created on: May 5, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */
#include "hardware.h"
#include "system.h"
#include "usart.h"
#include "wifi.h"

WifiUsartData_S wifiData;

WIFI_AP_SECRET_S wifiAPSecret = { DEFAULT_SSID, DEFAULT_PWD };

TaskHandle_t wifiAPTaskHandle;
EventGroupHandle_t wifiEventHandler = NULL;
SemaphoreHandle_t wifiBusySemaphoreMutexHandle = NULL;



/********wifi function start****************************************/

void wifi_hardware_start();
WIFI_UART_RSP_E wifi_send_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime);
WIFI_UART_RSP_E wifi_send_data(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime);
bool wifi_software_config(WIFI_NET_E wifiMode);
bool wifi_joinAP(char * pSSID, char * pPassWord);
WIFI_UART_RSP_E wifi_send_uart(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime, bool flag);
/********wifi function end******************************************/

void wifi_Handle()
{
	wifiData.index = 0;
	xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_UART_TC | EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_CONFIG);
	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);

	HAL_UART_Receive_DMA(wifiUart, wifiData.netUsartRxBuffer[!wifiData.index], DEBUG_RX_BUF_SIZE);
}

void wifi_AP_task()
{
	WIFI_AP_STATUS_E ap_status = CONFIG;

	wifi_Handle();

	Debug("start wifi ap task.\n");
	wifi_hardware_start();

	while (1)
	{
		switch (ap_status)
		{
		case CONFIG:

			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_CONFIG);
			wifi_software_config(STA);
			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_CONFIG);
			ap_status = DIS_CONNECT;
			break;
		case DIS_CONNECT:
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_DIS_AP);
			ap_status = CONNECTING;
			//TODO 断连状态，需要做什么操作？

			break;

		case CONNECTING:
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_CON_ING_AP);
			wifi_joinAP(wifiAPSecret.ssid, wifiAPSecret.pwd);
			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_CON_ING_AP);
			ap_status = CONNECTED;
			Debug("connected to AP\n");

			//TODO设置单/多连接，开启业务任务
			break;

		case CONNECTED:
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP);
			//TODO 心跳

			while (1)
			{
				Debug("wait hearbeat\n");
				vTaskDelay(10000);
			}
			ap_status = DIS_CONNECTING;
			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP);
			break;
		case DIS_CONNECTING:
			//TODO
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_DIS_ING_AP);
			ap_status = CONFIG;

			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_DIS_ING_AP);
			break;

		default:
			;
			//TODO
		}

	}
}

void wifi_hardware_start()
{
	uint8_t repeatTime = 0;

	HAL_GPIO_WritePin(WIFI_RST_GPIO_PORT, WIFI_RST_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(WIFI_EN_GPIO_PORT, WIFI_EN_PIN, GPIO_PIN_RESET);
	vTaskDelay(20);
	HAL_GPIO_WritePin(WIFI_EN_GPIO_PORT, WIFI_EN_PIN, GPIO_PIN_SET);
	vTaskDelay(100);
#if 1

	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
	while (1)
	{
		xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
		if (SUC == wifi_send_cmd("AT", "OK", NULL, WIFI_UART_WAITTIME))
		{
			repeatTime = 0;
			break;
		}
		vTaskDelay(20);
		if (repeatTime++ > 10)
		{
			return;
		}
	}
#endif

	wifi_send_cmd("ATE0", NULL, NULL, WIFI_UART_WAITTIME);
	vTaskDelay(20);
}

bool wifi_software_config(WIFI_NET_E wifiMode)
{
	uint8_t repeatTime = 0;
	char AT_TABLE[20] = { 0 };


	while (1)
	{
		sprintf(AT_TABLE, "AT+CWMODE=%d", wifiMode);
		xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);

		//TODO 待查询默认模式
		if (SUC == wifi_send_cmd(AT_TABLE, "OK", NULL, (WIFI_UART_WAITTIME * 2)))
		{
			repeatTime = 0;
			break;
		}
		vTaskDelay(500);
		if (repeatTime++ > 10)
		{
			return false;
		}
	}
	return true;

}

bool wifi_joinAP(char * pSSID, char * pPassWord)
{
	char cmd[120];
	WIFI_UART_RSP_E rsp = ERR;

	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);

	while (1)
	{
		xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
		rsp = wifi_send_cmd(cmd, "OK", "FAIL", (WIFI_UART_WAITTIME * 20));

		if (SUC == rsp)
		{
			Debug("join AP success!\n");
			return true;
		}
		else
		{
			printf("join AP failed, the error code is %d , re-connect\n", rsp);
		}
		vTaskDelay(1000);
	}

}

void distribute_msg(void)
{

#if 0
	wifi_Handle();
	wifi_hardware_start();
#endif
	ulTaskNotifyTake(pdTRUE, 0);
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#if 0
		printf("cmd:\n");
		printf(wifiData.netUsartRxBuffer[wifiData.index]);
		//printf(wifiData.netUsartRxBuffer[!wifiData.index]);
		printf("end\n");
#else

		if (strstr(wifiData.netUsartRxBuffer[wifiData.index], "WIFI DISCONNECT"))
		{
			//TODO
			xEventGroupClearBits(wifiEventHandler,
					EVENTBIT_WIFI_CONNECTED_AP | EVENTBIT_WIFI_LINK_0 | EVENTBIT_WIFI_LINK_1 | EVENTBIT_WIFI_LINK_2
							| EVENTBIT_WIFI_LINK_3 | EVENTBIT_WIFI_LINK_4);

			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_DIS_AP);

			Debug("system notify!\n");
		}
		else if (strstr(wifiData.netUsartRxBuffer[wifiData.index], "+IPD"))
		{
			//TODO收到数据，需要单独处理
		}
		else
		{
			printf(wifiData.netUsartRxBuffer[wifiData.index]);
			printf("\n");
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
		}
#endif
	}

}

WIFI_UART_RSP_E wifi_send_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, errInfo, waittime, true);

}

WIFI_UART_RSP_E wifi_send_data(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime)
{
	return wifi_send_uart(buffer, sucInfo, errInfo, waittime, false);
}

WIFI_UART_RSP_E wifi_send_uart(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime, bool flag)
{
	EventBits_t eventValue;
	uint8_t txData[WIFI_TX_BUF_SIZE] = { 0 };
	uint8_t endflag[2] = { 0x0d, 0x0a };

	uint32_t size = strlen(buffer);

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

	if ((sucInfo == NULL) && (errInfo == NULL))
	{
		return SUC;
	}

	eventValue = xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC, pdTRUE, pdFALSE, waittime);
	if ((eventValue & EVENTBIT_WIFI_UART_REC) == 0)
	{
		printf("WIFI UART get response overtime\n");
		return OVERTIME;
	}
	if ((sucInfo != NULL) && strstr(wifiData.netUsartRxBuffer[wifiData.index], sucInfo))
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
