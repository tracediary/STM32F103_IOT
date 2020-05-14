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
WIFI_UART_RSP_E wifi_send_link_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime);
WIFI_UART_RSP_E wifi_send_cmd_mux(char * buffer, char * sucInfo1, char * sucInfo2, char * errInfo, uint32_t waittime);

WIFI_UART_RSP_E wifi_send_data(char * buffer, char * sucInfo, char * errInfo, uint32_t waittime);
WIFI_UART_RSP_E wifi_send_ascii_data_cmd(char * buffer, char * sucInfo, char * errInfo, uint32_t len, uint32_t waittime);
WIFI_STATUS_E get_wifi_status();

extern uint8_t * wifi_uart_curdata_addr;
extern WifiLinkInfo_S wifiLinkdata;
extern const uint32_t wifi_link_ready[WIFI_MAX_LINK];
#endif /* USERINC_WIFIUTIL_H_ */
