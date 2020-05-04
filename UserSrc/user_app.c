/*
 * user_app.c
 *
 *  Created on: 2020年5月4日
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv2, free software
 */

#include "debug.h"
#include "constant.h"
#include "hardware.h"

/**FreeRTOS head file**********************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"

EventGroupHandle_t wifiEventHandler = NULL;
EventGroupHandle_t debugEventHandler = NULL;

SemaphoreHandle_t wifiBusySemaphoreMutexHandle;

TaskHandle_t WIFI_Handle = NULL, IWDG_Handle = NULL, MQTT_Handle = NULL;


//this is the app start
void app_os_var_init()
{

}

void app_task_init(void)
{


}
