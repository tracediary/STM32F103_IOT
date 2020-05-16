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
#include "wifiUtil.h"
#include "transport.h"
#include "sys_util.h"
#include "time_transform.h"
#include "rtc.h"
/********wifi function start****************************************/

void wifi_hardware_start();
bool wifi_software_config(WIFI_NET_E wifiMode);
bool wifi_joinAP(char * pSSID, char * pPassWord);
bool wifi_set_mux();
void wifi_restart();
WIFI_UART_RSP_E wifi_send_uart(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime, bool flag);
void wifi_mqtt_link_event_callback();
void wifi_sntp_link1_event_callback();
void wifi_tcp_link2_event_callback();
void wifi_udp_link1_event_callback();
void wifi_udp_link2_event_callback();
bool syncTime(uint8_t *timeBuf, int timeZone);
void setRtcTime();
void SntpSyncTimeCallback(void const * argument);
/********wifi function end******************************************/

/*******FreeRTOS para start**********************/
TaskHandle_t wifiAPTaskHandle = NULL;
EventGroupHandle_t wifiEventHandler = NULL;
SemaphoreHandle_t wifiBusySemaphoreMutexHandle = NULL;
SemaphoreHandle_t wifiSemaphoreMutexHandle = NULL;

TimerHandle_t SNTP_TimerHandle = NULL;

extern TaskHandle_t KEEP_Handle;
extern TaskHandle_t SNTP_Handle;
extern EventGroupHandle_t sysEventHandler;
/*******FreeRTOS para end**********************/

/********wifi para start**********************/
WifiUsartData_S wifiData;
uint8_t * wifi_uart_curdata_addr;

WIFI_AP_SECRET_S wifiAPSecret = { DEFAULT_SSID, DEFAULT_PWD };

WifiLinkInfo_S wifiLinkdata;

const uint32_t wifi_link_ready[WIFI_MAX_LINK] = { EVENTBIT_WIFI_LINK_0_READY, EVENTBIT_WIFI_LINK_1_READY,
EVENTBIT_WIFI_LINK_2_READY, EVENTBIT_WIFI_LINK_3_READY, EVENTBIT_WIFI_LINK_4_READY };

const uint32_t wifi_link[WIFI_MAX_LINK] = { EVENTBIT_WIFI_LINK_0, EVENTBIT_WIFI_LINK_1, EVENTBIT_WIFI_LINK_2,
EVENTBIT_WIFI_LINK_3, EVENTBIT_WIFI_LINK_4 };

const void (*link_Callback[WIFI_MAX_LINK])(void)=
{
	[WIFI_MQTT_LINK_ID] = wifi_mqtt_link_event_callback,
	[WIFI_SNTP_LINK_ID] = wifi_sntp_link1_event_callback,
	[WIFI_TCP2_LINK_ID] = wifi_tcp_link2_event_callback,
	[WIFI_UDP1_LINK_ID] = wifi_udp_link1_event_callback,
	[WIFI_UDP2_LINK_ID] = wifi_udp_link2_event_callback
};

/********wifi para end**********************/

void wifi_Handle()
{
	wifiData.index = 0;
	xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_UART_TC | EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_CONFIG);
	xEventGroupClearBits(wifiEventHandler,
			EVENTBIT_WIFI_UART_REC | EVENTBIT_WIFI_LINK_0 | EVENTBIT_WIFI_LINK_1 | EVENTBIT_WIFI_LINK_2
					| EVENTBIT_WIFI_LINK_3 | EVENTBIT_WIFI_LINK_4);

	HAL_UART_Receive_DMA(wifiUart, wifiData.netUsartRxBuffer[!wifiData.index], WIFI_RX_BUF_SIZE);
	wifi_uart_curdata_addr = wifiData.netUsartRxBuffer[wifiData.index];

	SNTP_TimerHandle = xTimerCreate("SNTP_circle_timer", (DELAY_BASE_MIN_TIME * 60 * 24), pdTRUE, (void *) 2,
			SntpSyncTimeCallback);

}

void wifi_AP_task()
{
	WIFI_AP_STATUS_E ap_status = CONFIG;
	EventBits_t eventValue;

	wifi_Handle();

	WIFI_START:

	Debug("start wifi ap task.\n");
	wifi_hardware_start();

	while (1)
	{
		switch (ap_status)
		{
		case CONFIG:

			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_CONFIG);
			if (!wifi_software_config(STA))
			{
				goto WIFI_START;
			}

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

			if (!wifi_joinAP(wifiAPSecret.ssid, wifiAPSecret.pwd))
			{
				vTaskDelay(DELAY_BASE_SEC_TIME * 5);
				break;
			}

			ap_status = CONNECTED;
			Debug("connected to AP\n");
			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_CON_ING_AP);

			break;

		case CONNECTED:

			if (!wifi_set_mux())
			{
				vTaskDelay(DELAY_BASE_SEC_TIME);
				break;
			}

			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP);

			while (1)
			{
				eventValue = xEventGroupWaitBits(wifiEventHandler, (EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_DIS_ING_AP),
						pdFALSE, pdFALSE, portMAX_DELAY);
				if (((eventValue & EVENTBIT_WIFI_DIS_ING_AP) != 0) || ((eventValue & EVENTBIT_WIFI_DIS_AP) != 0))
				{
					printf("wifi disconnect, re-connect!\n");
					break;
				}
				Debug("nothing happened!\n");
			}
			ap_status = DIS_CONNECTING;
			xEventGroupClearBits(wifiEventHandler,
					EVENTBIT_WIFI_CONNECTED_AP | EVENTBIT_WIFI_LINK_0 | EVENTBIT_WIFI_LINK_1 | EVENTBIT_WIFI_LINK_2
							| EVENTBIT_WIFI_LINK_3 | EVENTBIT_WIFI_LINK_4);
			break;
		case DIS_CONNECTING:
			//TODO wifi dis connect,如果是透传模式，要退出透传在发送断连指令，然后重启wifi，暂时硬件重启
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_DIS_ING_AP);
			ap_status = CONFIG;
//			wifi_restart();
			xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_DIS_ING_AP);
//			goto WIFI_START;
			break;

		default:
			ap_status = CONFIG;
			printf("erron status, go to config\n");
			break;
		}

	}
}

void wifi_restart()
{
	vTaskDelay(1000);
	Debug("go to restart wifi\n");
	wifi_send_cmd("AT+RST", "OK", NULL, (WIFI_UART_WAITTIME * 4));
	vTaskDelay(3000);
}

void wifi_hardware_start()
{
	uint8_t repeatTime = 0;

	HAL_GPIO_WritePin(WIFI_RST_GPIO_PORT, WIFI_RST_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(WIFI_EN_GPIO_PORT, WIFI_EN_PIN, GPIO_PIN_RESET);
	vTaskDelay(DELAY_BASE_10MIL_TIME * 10);
	HAL_GPIO_WritePin(WIFI_EN_GPIO_PORT, WIFI_EN_PIN, GPIO_PIN_SET);
	vTaskDelay(DELAY_BASE_10MIL_TIME * 50);

	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
	while (1)
	{
		xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
		if (SUC == wifi_send_cmd("AT", "OK", NULL, WIFI_UART_WAITTIME))
		{
			repeatTime = 0;
			break;
		}
		vTaskDelay(DELAY_BASE_SEC_TIME);
		if (repeatTime++ > 5)
		{
			return;
		}
	}

	wifi_send_cmd("ATE0", NULL, NULL, WIFI_UART_WAITTIME);
	vTaskDelay(DELAY_BASE_SEC_TIME);
}

bool wifi_software_config(WIFI_NET_E wifiMode)
{
	uint8_t repeatTime = 0;
	char AT_TABLE[20] = { 0 };

	while (1)
	{
		sprintf(AT_TABLE, "AT+CWMODE=%d", wifiMode);

		//TODO 待查询默认模式
		if (SUC == wifi_send_cmd(AT_TABLE, "OK", NULL, (WIFI_UART_WAITTIME * 2)))
		{
			printf("set wifi cwmode single link\n");
			break;
		}
		vTaskDelay(DELAY_BASE_SEC_TIME);
		if (repeatTime++ > 5)
		{
			Error("set cwmode failed\n");
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

	rsp = wifi_send_cmd(cmd, "OK", "+CWJAP", (WIFI_UART_WAITTIME * 30));

	if (SUC == rsp)
	{
		Debug("join AP success!\n");
		return true;
	}
	else
	{
		printf("join AP failed, the error code is %d , re-connect\n", rsp);
		return false;
	}

}

bool wifi_set_mux()
{
	WIFI_UART_RSP_E rsp = ERR;

	rsp = wifi_send_cmd("AT+CIPMUX=1", "OK", "ERROR", (WIFI_UART_WAITTIME * 2));

	if (SUC == rsp)
	{
		Debug("set wifi work in mux link mode\n");
		return true;
	}
	else
	{
		Error("set wifi work in mux failed, the error code is %d , reset\n", rsp);
		return false;
	}

}

void sntp_rtc_task()
{
	while (1)
	{
		xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP, pdFALSE, pdFALSE, portMAX_DELAY);
		Debug("start to sync time\n");

		wifiLinkdata.data_buf[WIFI_SNTP_LINK_ID] = pvPortMalloc(WIFI_NORMAL_BUF_SIZE * 2);
		if (wifiLinkdata.data_buf[WIFI_SNTP_LINK_ID] == NULL)
		{
			Error("%s apply ram for SNTP data buffer failed!\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));
			vTaskDelay(DELAY_BASE_MIN_TIME);

			continue;
		}
		wifiLinkdata.maxLenth[WIFI_SNTP_LINK_ID] = WIFI_NORMAL_BUF_SIZE * 2;

		setRtcTime();

		vPortFree(wifiLinkdata.data_buf[WIFI_SNTP_LINK_ID]);
		Debug("end to sync time\n");

		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	}
}

void setRtcTime()
{
	uint8_t buf[WIFI_NORMAL_BUF_SIZE] = { 0 };
	BaseType_t err = pdFALSE;
	EventBits_t eventValue;


	SET_RTC_TIME:

	err = xSemaphoreTake(wifiSemaphoreMutexHandle, (WIFI_UART_WAITTIME * 10));
	if (err == pdFALSE)
	{
		Debug("get wifiSemaphoreMutexHandle failed, wait next time\n");
		goto SYS_EVENT;
	}

	if (!syncTime(buf, TIME_ZONE))
	{
		xSemaphoreGive(wifiSemaphoreMutexHandle);
		goto SYS_EVENT;
	}

	if (!RTC_Set_RealTime(buf))
	{
		xSemaphoreGive(wifiSemaphoreMutexHandle);
		goto SYS_EVENT;
	}

	xEventGroupSetBits(sysEventHandler, EVENTBIT_SYS_SYNC_TIME);
	xSemaphoreGive(wifiSemaphoreMutexHandle);
	return;

	SYS_EVENT:

	eventValue = xEventGroupGetBits(sysEventHandler);
	if ((eventValue & EVENTBIT_SYS_SYNC_TIME) != 0)
	{
		return;
	}
	//1 minute later, sync time again

	vTaskDelay(DELAY_BASE_MIN_TIME);
	goto SET_RTC_TIME;


}

bool syncTime(uint8_t *timeBuf, int timeZone)
{
	char cmd[WIFI_NORMAL_BUF_SIZE * 2] = { 0 };
	EventBits_t eventValue = 0;

	if (SUC != waitWifiAvailable())
	{
		return false;
	}

	sprintf(cmd, "AT+CIPSTART=%d,\"TCP\",\"time.nist.gov\",13", WIFI_SNTP_LINK_ID);

	xEventGroupClearBits(wifiEventHandler, wifi_link_ready[WIFI_SNTP_LINK_ID]);
	wifi_send_cmd(cmd, NULL, NULL, 0);

	eventValue = xEventGroupWaitBits(wifiEventHandler, wifi_link_ready[WIFI_SNTP_LINK_ID], pdTRUE, pdFALSE,
			(WIFI_UART_WAITTIME * 4));
	if ((eventValue & wifi_link_ready[WIFI_SNTP_LINK_ID]) == 0)
	{
		Error("sync time failed, wait next time\n");
		return false;
	}

	strncpy(timeBuf, (strstr(wifiLinkdata.data_buf[WIFI_SNTP_LINK_ID], " ") + 1), (WIFI_NORMAL_BUF_SIZE - 1));

	GetDateTimeFromUTC(timeBuf, timeZone);

	return true;

}

void distribute_msg(void)
{
	uint8_t * wifi_uart_str = NULL;
	uint32_t len = 0;
	uint8_t link_ID = 0;
	uint8_t i = 0;

	ulTaskNotifyTake(pdTRUE, 0);
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (strstr(wifi_uart_curdata_addr, "+IPD"))
		{
			//Debug("get the net data: %s .\n", wifi_uart_curdata_addr);

			wifi_uart_str = strstr(wifi_uart_curdata_addr, "+IPD") + 5;

			link_ID = 0;
			while (',' != *wifi_uart_str)
			{
				link_ID = link_ID * 10 + (*wifi_uart_str - '0');
				wifi_uart_str++;
			}

			if (link_ID > ( WIFI_MAX_LINK - 1))
			{
				Error("link id error, desert data\n");
				continue;
			}
			if (wifiLinkdata.data_buf[link_ID] == NULL)
			{
				Error("this link have not data buffer, desert data\n");
				continue;
			}
			//get remain data lenth;
			wifi_uart_str++;
			len = 0;
			while (':' != *wifi_uart_str)
			{
				len = len * 10 + (*wifi_uart_str - '0');
				wifi_uart_str++;
			}

			if (len > (wifiLinkdata.maxLenth - 1))
			{
				Error("this data too long than the buffer, desert data\n");
				continue;
			}

			wifi_uart_str++;

			memset(wifiLinkdata.data_buf[link_ID], 0, (len + 1));

			memcpy(wifiLinkdata.data_buf[link_ID], wifi_uart_str, len);
			wifiLinkdata.lenth[link_ID] = len;

			xEventGroupSetBits(wifiEventHandler, wifi_link_ready[link_ID]);

			if (startWith((wifi_uart_curdata_addr + 2), "+IPD,"))
			{
				Debug("++++++++++++++++++++++++++++++\n");
				continue;
			}
			//HAL_UART_Transmit_DMA(debugUart, wifiLinkdata.data_buf[link_ID], len);
		}

		if (strstr(wifi_uart_curdata_addr, ",CLOSED"))
		{
			wifi_uart_str = wifi_uart_curdata_addr;

			link_ID = 0;
			while (',' != *wifi_uart_str)
			{
				link_ID = link_ID * 10 + (*wifi_uart_str - '0');
				wifi_uart_str++;
			}

			if (link_ID > ( WIFI_MAX_LINK - 1))
			{
				Error("link id error, desert data\n");
				continue;
			}

			Debug("link %d closed\n", link_ID);
			xEventGroupClearBits(wifiEventHandler, wifi_link[link_ID]);
			link_Callback[link_ID];
		}
		else if (strstr(wifi_uart_curdata_addr, "WIFI DISCONNECT"))
		{
			//TODO wifi断连，重连
			xEventGroupClearBits(wifiEventHandler,
					EVENTBIT_WIFI_CONNECTED_AP | EVENTBIT_WIFI_LINK_0 | EVENTBIT_WIFI_LINK_1 | EVENTBIT_WIFI_LINK_2
							| EVENTBIT_WIFI_LINK_3 | EVENTBIT_WIFI_LINK_4);

			xEventGroupSetBits(wifiEventHandler, (EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_DIS_ING_AP));

			for (i = 0; i < WIFI_MAX_LINK; i++)
			{
				link_Callback[i];
			}

			printf("system notify, wifi dis-connect to AP!\n");
		}
		else
		{
			//printf("get uart cmd ack: ");
			//printf(wifi_uart_curdata_addr);
			//printf("\n");
			xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_UART_REC);
		}

//		if (strstr(wifi_uart_curdata_addr, "SEND OK") || strstr(wifi_uart_curdata_addr, "SEND FAIL")
//				|| strstr(wifi_uart_curdata_addr, "ERROR"))

	}

}

void SntpSyncTimeCallback(void const * argument)
{
	xTaskNotifyGive(SNTP_Handle);
}


void wifi_mqtt_link_event_callback()
{
	xTaskNotifyGive(KEEP_Handle);
}
void wifi_sntp_link1_event_callback()
{
	;
}
void wifi_tcp_link2_event_callback()
{
	;
}
void wifi_udp_link1_event_callback()
{
	;
}
void wifi_udp_link2_event_callback()
{
	;
}

