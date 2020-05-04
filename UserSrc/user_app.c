/*
 * user_app.c
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

/***FreeRTOS var define**************************************************/
EventGroupHandle_t wifiEventHandler = NULL;
EventGroupHandle_t debugEventHandler = NULL;

SemaphoreHandle_t wifiBusySemaphoreMutexHandle;

TaskHandle_t WIFI_Handle = NULL, IWDG_Handle = NULL, TCP_Handle = NULL, MQTT_Handle = NULL;

void wifi_task(void *pvParameters);
void tcp_task(void *pvParameters);
/**user var define****************************************/






void app_os_var_init()
{
	wifiEventHandler = xEventGroupCreate();
	debugEventHandler = xEventGroupCreate();

	wifiBusySemaphoreMutexHandle = xSemaphoreCreateMutex();

}

void app_task_init(void)
{

	xTaskCreate(wifi_task, "wifiTask", (configMINIMAL_STACK_SIZE * 5), NULL, (tskIDLE_PRIORITY + 10), &WIFI_Handle);
	xTaskCreate(tcp_task, "tcpTask", (configMINIMAL_STACK_SIZE * 10), NULL, (tskIDLE_PRIORITY + 9), &TCP_Handle);

}

void wifi_task(void *pvParameters)
{
	while (1)
	{
		vTaskDelay(2000);
	}

}

void tcp_task(void *pvParameters)
{
	while (1)
	{
		vTaskDelay(2000);
	}
}

void iwdgTask(void *pvParameters)
{
	while (1)
	{
	}

}


