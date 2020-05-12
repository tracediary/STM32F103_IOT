/*
 * wifiUtil.h
 *
 *  Created on: May 7, 2020
 *      Author: Jason C
 *     version: V1.0.1
 ************************************************************************
 *	 attention: GPLv3, free software
 */

#ifndef USERINC_WIFIUTIL_H_
#define USERINC_WIFIUTIL_H_

#include "system.h"

WIFI_UART_RSP_E wifi_send_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime);
WIFI_UART_RSP_E wifi_send_cmd_mux(char * buffer, char * sucInfo1, char * sucInfo2, char * errInfo, uint32_t waittime);
WIFI_UART_RSP_E wifi_send_data(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime);
void wifi_send_mqtt_data(char * buffer, char * sucInfo, char * errInfo, uint32_t len, uint32_t waittime);

#endif /* USERINC_WIFIUTIL_H_ */
