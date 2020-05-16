/*
 * user_app.c
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#include "hardware.h"
#include "system.h"
#include "wifi.h"
#include "usart.h"
#include "mqttclient.h"


TaskHandle_t WIFI_Handle = NULL, IWDG_Handle = NULL, SNTP_Handle = NULL, MQTT_Handle = NULL, DIST_Handle = NULL;
EventGroupHandle_t sysEventHandler = NULL;



extern EventGroupHandle_t debugEventHandler;
void wifi_task(void *pvParameters);
void sntp_task(void *pvParameters);
void wifi_uart_msg_distribute_task(void *pvParameters);
/**user var define****************************************/

void app_os_var_init()
{
	wifiEventHandler = xEventGroupCreate();
	sysEventHandler = xEventGroupCreate();
	wifiBusySemaphoreMutexHandle = xSemaphoreCreateMutex();

}

void app_task_init(void)
{
	xTaskCreate(wifi_task, "wifiTask", (configMINIMAL_STACK_SIZE * 8), NULL, (tskIDLE_PRIORITY + 5), &WIFI_Handle);

	xTaskCreate(wifi_uart_msg_distribute_task, "distributeTask", (configMINIMAL_STACK_SIZE * 8), NULL,
			(tskIDLE_PRIORITY + 5), &DIST_Handle);
	mqtt_thread_init();

	xTaskCreate(sntp_task, "tcpTask", (configMINIMAL_STACK_SIZE * 2), NULL, (tskIDLE_PRIORITY + 1), &SNTP_Handle);

}

void wifi_task(void *pvParameters)
{
	while (1)
	{
		wifi_AP_task();


		vTaskDelay(2000);
	}

}

void wifi_uart_msg_distribute_task(void *pvParameters)
{
	distribute_msg();
}

void sntp_task(void *pvParameters)
{
	sntp_rtc_task();
}

void iwdgTask(void *pvParameters)
{
	while (1)
	{
	}

}

