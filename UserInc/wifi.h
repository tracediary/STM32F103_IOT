/*
 * wifi.h
 *
 *  Created on: May 5, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#ifndef USERINC_WIFI_H_
#define USERINC_WIFI_H_

#include "system.h"

extern EventGroupHandle_t wifiEventHandler;
extern SemaphoreHandle_t wifiBusySemaphoreMutexHandle;
extern TaskHandle_t wifiAPTaskHandle;
extern SemaphoreHandle_t wifiSemaphoreMutexHandle;


void wifi_Handle();

void wifi_AP_task();
void distribute_msg(void);


#endif /* USERINC_WIFI_H_ */
