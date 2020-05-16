/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/


#include "system.h"
#if !defined(SOCKET_ERROR)
/** error in socket operation */
#define SOCKET_ERROR -1
#endif


#define INVALID_SOCKET SOCKET_ERROR
#if 0
#include <sys/socket.h>
#endif



#if 0
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#if 0
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

#include <stdlib.h>
#include "transport.h"
#include "wifiUtil.h"

/**
 This simple low-level implementation assumes a single connection for a single thread. Thus, a static
 variable is used for that connection.
 On other scenarios, the user must solve this by taking into account that the current implementation of
 MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
 to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
 */

extern WifiUsartData_S wifiData;
extern EventGroupHandle_t wifiEventHandler;
extern uint32_t first_index_to_read;

int transport_sendPacketBuffer(unsigned char* buf, int buflen)
{
	int rc = 0;
	char cmd[20] = { 0 };
	WIFI_UART_RSP_E rsp = ERR;
	sprintf(cmd, "AT+CIPSEND=%d,%d", WIFI_MQTT_LINK_ID, buflen);

	rsp = wifi_send_cmd(cmd, ">", NULL, (WIFI_UART_WAITTIME * 2));
	if (rsp != SUC)
	{
		return -1;
	}

	rsp = wifi_send_ascii_data_cmd(buf, "SEND OK", "SEND FAIL", buflen, (WIFI_UART_WAITTIME * 8));
//	rsp = wifi_send_ascii_data_cmd(buf, NULL, NULL, buflen, WIFI_UART_WAITTIME);
	if (rsp != SUC)
	{
		return -2;
	}
	return 1;
}

int transport_getdata(unsigned char* buf, int count)
{
	memcpy(buf, &(wifiLinkdata.data_buf[WIFI_MQTT_LINK_ID][first_index_to_read]), count);
//	memcpy(buf, (wifiLinkdata.data_buf[WIFI_MQTT_LINK_ID] + first_index_to_read), count);
	first_index_to_read += count;
//	int rc = recv(mysock, buf, count, 0);
	//printf("received %d bytes count %d\n", rc, (int)count);
	return count;
}


/**
 * open a link to server
 * char* addr: server IP or domain
 * int	 port: server port
 * <0 add tcp link failed
 */
int transport_open(char* addr, int port)
{
	//TODO 建立TCP链接，待定是否判断多连接，设置返回端口号
	char cmd[100] = { 0 };
	WIFI_UART_RSP_E rsp = ERR;

	sprintf(cmd, "AT+CIPSTART=%d,\"TCP\",\"%s\",%d", WIFI_MQTT_LINK_ID, addr, port);
	while (1)
	{
		rsp = wifi_send_cmd_mux(cmd, "OK", "ALREADY CONNECTED", "ERROR", (WIFI_UART_WAITTIME * 30));
		if (SUC == rsp)
		{
			Debug("connect to server\n");
			break;
		}
		else
		{
			printf("connect to server failed, the error code is %d, re-connect\n", rsp);
		}
		vTaskDelay(1000);
	}

#if 0
	//透传模式有断连bug，暂时不用透传
	while (1)
	{
		rsp = wifi_send_cmd("AT+CIPMODE=1", "OK", "ERROR", WIFI_UART_WAITTIME);
		if (SUC == rsp)
		{
			Debug("set the WIFI single mode success\n");
			break;
		}
		else
		{
			printf("set the WIFI single mode failed, the error code is %d\n", rsp);
		}
		vTaskDelay(1000);
	}
	xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_SINGLE_MODE);
	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_MUX);
	wifi_send_cmd("AT+CIPSEND", ">", NULL, WIFI_UART_WAITTIME);
#endif

	return WIFI_MQTT_LINK_ID;
}

int transport_close(int sock)
{
	//TODO 没有判断是否透传模式,多连接模式下，关闭连接也需要修改
	WIFI_STATUS_E wifi_status = DIS_CON_AP;
	char cmd[20] = { 0 };
//	wifi_send_mqtt_data("+++", NULL, NULL, 3, WIFI_UART_WAITTIME);
//	vTaskDelay(2000);
//	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_SINGLE_MODE);


	wifi_status = get_wifi_status();
	switch (wifi_status)
	{
	case CON_AP:
		break;
	case CON_TCP_UDP:
		Debug("sys want to close mqtt link\n");
		sprintf(cmd, "AT+CIPCLOSE=%d", WIFI_MQTT_LINK_ID);
		wifi_send_cmd(cmd, "OK", "ERROR", (WIFI_UART_WAITTIME * 2));
		break;
	case DIS_CON_TCP_UDP:
		break;
	case DIS_CON_AP:
		xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP);
		xEventGroupSetBits(wifiEventHandler, (EVENTBIT_WIFI_DIS_AP | EVENTBIT_WIFI_DIS_ING_AP));

		break;
	default:
		break;
	}

	return 0;
}

