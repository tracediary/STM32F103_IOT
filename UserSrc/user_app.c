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


TaskHandle_t WIFI_Handle = NULL, IWDG_Handle = NULL, TCP_Handle = NULL, MQTT_Handle = NULL, DIST_Handle = NULL;
extern EventGroupHandle_t debugEventHandler;
void wifi_task(void *pvParameters);
void tcp_task(void *pvParameters);
void wifi_uart_msg_distribute_task(void *pvParameters);
/**user var define****************************************/

void app_os_var_init()
{
	wifiEventHandler = xEventGroupCreate();
	wifiBusySemaphoreMutexHandle = xSemaphoreCreateMutex();

}

void app_task_init(void)
{
	xTaskCreate(wifi_task, "wifiTask", (configMINIMAL_STACK_SIZE * 8), NULL, (tskIDLE_PRIORITY + 5), &WIFI_Handle);

	xTaskCreate(wifi_uart_msg_distribute_task, "distributeTask", (configMINIMAL_STACK_SIZE * 8), NULL,
			(tskIDLE_PRIORITY + 5), &DIST_Handle);
	mqtt_thread_init();

	//xTaskCreate(tcp_task, "tcpTask", (configMINIMAL_STACK_SIZE * 4), NULL, (tskIDLE_PRIORITY + 5), &TCP_Handle);

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

void tcp_task(void *pvParameters)
{

	xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP, pdFALSE, pdFALSE, portMAX_DELAY);

	taskENTER_CRITICAL();

	//mqtt_thread_init();

	vTaskDelete(TCP_Handle);
	taskEXIT_CRITICAL();
}

void iwdgTask(void *pvParameters)
{
	while (1)
	{
	}

}

