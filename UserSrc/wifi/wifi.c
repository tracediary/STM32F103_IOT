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

#include "transport.h"

WifiUsartData_S wifiData;

WIFI_AP_SECRET_S wifiAPSecret = { DEFAULT_SSID, DEFAULT_PWD };

TaskHandle_t wifiAPTaskHandle = NULL;
EventGroupHandle_t wifiEventHandler = NULL;
SemaphoreHandle_t wifiBusySemaphoreMutexHandle = NULL;

extern TaskHandle_t KEEP_Handle;

uint8_t rec_data[WIFI_RX_BUF_SIZE];

/********wifi function start****************************************/

void wifi_hardware_start();
bool wifi_software_config(WIFI_NET_E wifiMode);
bool wifi_joinAP(char * pSSID, char * pPassWord);
WIFI_UART_RSP_E wifi_send_uart(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime, bool flag);
/********wifi function end******************************************/

void wifi_Handle()
{
	wifiData.index = 0;
	xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_UART_TC | EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_CONFIG);
	xEventGroupClearBits(wifiEventHandler,
			EVENTBIT_WIFI_UART_REC | EVENTBIT_WIFI_LINK_0 | EVENTBIT_WIFI_LINK_1 | EVENTBIT_WIFI_LINK_2
					| EVENTBIT_WIFI_LINK_3 | EVENTBIT_WIFI_LINK_4);

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
				//Debug("wait hearbeat\n");
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
	vTaskDelay(100);
	HAL_GPIO_WritePin(WIFI_EN_GPIO_PORT, WIFI_EN_PIN, GPIO_PIN_SET);
	vTaskDelay(500);

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
	char * len_str = NULL;
	uint32_t len = 0;

	ulTaskNotifyTake(pdTRUE, 0);
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (strstr(wifiData.netUsartRxBuffer[wifiData.index], "WIFI DISCONNECT"))
		{
			//TODO wifi断连，重连
			xEventGroupClearBits(wifiEventHandler,
					EVENTBIT_WIFI_CONNECTED_AP | EVENTBIT_WIFI_LINK_0 | EVENTBIT_WIFI_LINK_1 | EVENTBIT_WIFI_LINK_2
							| EVENTBIT_WIFI_LINK_3 | EVENTBIT_WIFI_LINK_4);

			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_DIS_AP);
			xTaskNotifyGive(KEEP_Handle);

			Debug("system notify!\n");
		}
		else if (strstr(wifiData.netUsartRxBuffer[wifiData.index], "+IPD"))
		{
			//TODO收到数据，需要单独处理,非透传模式

			memset(rec_data, 0, WIFI_RX_BUF_SIZE);

			len_str = strstr(wifiData.netUsartRxBuffer[wifiData.index], ",") + 1;

			len = 0;
			while (':' != *len_str)
			{
				len = len * 10 + (*len_str - '0');
				len_str++;
			}

			memcpy(rec_data, (len_str++), len);

			printf("get the net data: %s, the len is %d\n", rec_data, len);

		}
		else if (strstr(wifiData.netUsartRxBuffer[wifiData.index], "CLOSED"))
		{
			//TODO 透传模式下断连
			Debug("MQTT link closed\n");
			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_LINK_0);
			xTaskNotifyGive(KEEP_Handle);
		}
		else
		{
			printf(wifiData.netUsartRxBuffer[wifiData.index]);
			printf("\n");
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
		}
	}

}
