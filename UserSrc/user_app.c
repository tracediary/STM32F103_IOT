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

	xTaskCreate(wifi_task, "wifiTask", (configMINIMAL_STACK_SIZE * 8), NULL, (tskIDLE_PRIORITY + 3), &WIFI_Handle);

	xTaskCreate(wifi_uart_msg_distribute_task, "distributeTask", (configMINIMAL_STACK_SIZE * 4), NULL,
			(tskIDLE_PRIORITY + 5), &DIST_Handle);
	//xTaskCreate(tcp_task, "tcpTask", (configMINIMAL_STACK_SIZE * 8), NULL, (tskIDLE_PRIORITY + 5), &TCP_Handle);

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
	int i = 0;
	uint8_t data[100] = { 0 };
	while (1)
	{
		Debug("this is the debug info\n");
		sprintf(data, "this is the DMA %d time\n", i++);
		xEventGroupClearBits(debugEventHandler, EVENTBIT_DEBUG_UART_TC);
		HAL_UART_Transmit_DMA(debugUart, data, strlen(data));

		Debug("info from print \n");
		vTaskDelay(1000);
	}
}

void iwdgTask(void *pvParameters)
{
	while (1)
	{
	}

}

