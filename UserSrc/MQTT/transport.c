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
//	int rc = 0;
	wifi_send_mqtt_data(buf, NULL, NULL, buflen, WIFI_UART_WAITTIME);
	return 1;
}

int transport_getdata(unsigned char* buf, int count)
{
	memcpy(buf, &(wifiData.netUsartRxBuffer[wifiData.index][first_index_to_read]), count);
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

	sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d", addr, port);
	while (1)
	{
		rsp = wifi_send_cmd_mux(cmd, "OK", "ALREAY CONNECT", "ERROR", (WIFI_UART_WAITTIME * 10));
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

	wifi_send_cmd("AT+CIPSEND", ">", NULL, WIFI_UART_WAITTIME);

	xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_SINGLE_MODE);
	xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_MUX);
	return 0;
}

int transport_close(int sock)
{
	wifi_send_data("+++", NULL, NULL, WIFI_UART_WAITTIME);
	vTaskDelay(1500);
	wifi_send_cmd("AT+CIPCLOSE", NULL, NULL, WIFI_UART_WAITTIME);
	vTaskDelay(4000);

	return 0;
}
