#include "mqttclient.h"
#include "transport.h"
#include "MQTTPacket.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "string.h"
#include "cJSON_Process.h"

#include "wifi.h"
#include "usart.h"
#include "system.h"
#include "wifiUtil.h"

extern QueueHandle_t MQTT_Data_Queue;

MQTT_USER_MSG mqtt_user_msg;

int32_t MQTT_Socket = 0;

/**FreeRTOS define start**************************************/
TaskHandle_t RECV_Handle = NULL, SEND_Handle = NULL, KEEP_Handle = NULL;
TimerHandle_t AR_Mqtt_HB_TimerHandle = NULL;

/**FreeRTOS define end*************************************/
void deliverMessage(MQTTString *TopicName, MQTTMessage *msg, MQTT_USER_MSG *mqtt_user_msg);

uint32_t first_index_to_read = 0;

/************************************************************************
 ** 函数名称: MQTT_Connect
 ** 函数功能: 初始化客户端并登录服务器
 ** 入口参数: int32_t sock:网络描述符
 ** 出口参数: >=0:发送成功 <0:发送失败
 ** 备    注:
 ************************************************************************/
uint8_t MQTT_Connect(void)
{
	EventBits_t eventValue;
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	uint8_t buf[200] = { 0 };
	int buflen = sizeof(buf);
	int len = 0;

	unsigned char sessionPresent = 0;
	unsigned char connack_rc = 0;

	data.clientID.cstring = CLIENT_ID;
	data.keepAliveInterval = KEEPLIVE_TIME;
	data.username.cstring = USER_NAME;
	data.password.cstring = PASSWORD;
	data.MQTTVersion = MQTT_VERSION;
	data.cleansession = 1;
	len = MQTTSerialize_connect((unsigned char *) buf, buflen, &data);

	//Debug("send auth info to aliyun: %x, data len id %d\n", buf, len);
	//HAL_UART_Transmit_DMA(debugUart, buf, len);

	transport_sendPacketBuffer(buf, len);

	eventValue = xEventGroupWaitBits(wifiEventHandler, wifi_link_ready[WIFI_MQTT_LINK_ID], pdTRUE, pdFALSE,
			(WIFI_UART_WAITTIME * 10));
	if ((eventValue & wifi_link_ready[WIFI_MQTT_LINK_ID]) == 0)
	{
		printf("wait MQTT server response overtime\n");
	}
	memset(buf, 0, buflen);

	first_index_to_read = 0;
	if (MQTTPacket_read(buf, buflen, transport_getdata) == CONNACK)
	{

		if (MQTTDeserialize_connack(&sessionPresent, &connack_rc, buf, buflen) != 1 || connack_rc != 0)
		{
			Debug("Connect_NOK, the rc is %d\n", connack_rc);
			return Connect_NOK;
		}
		else
		{
			Debug("MQTT connect to server\n");
			return Connect_OK;
		}
	}
	else
		;
	return Connect_NOTACK;
}

/************************************************************************
 ** 函数名称: MQTT_PingReq
 ** 函数功能: 发送MQTT心跳包
 ** 入口参数: 无
 ** 出口参数: >=0:发送成功 <0:发送失败
 ** 备    注:
 ************************************************************************/
int32_t MQTT_PingReq(int32_t sock)
{
	int32_t len;
	uint8_t buf[100];
	int32_t buflen = sizeof(buf);
	EventBits_t eventValue;

	len = MQTTSerialize_pingreq(buf, buflen);
	transport_sendPacketBuffer(buf, len);

	eventValue = xEventGroupWaitBits(wifiEventHandler, wifi_link_ready[WIFI_MQTT_LINK_ID], pdTRUE, pdFALSE,
			(WIFI_UART_WAITTIME * 4));

	if ((eventValue & wifi_link_ready[WIFI_MQTT_LINK_ID]) == 0)
	{
		printf("wait ping req reply failed\n");
		return -2;
	}

	first_index_to_read = 0;
	if (MQTTPacket_read(buf, buflen, transport_getdata) != PINGRESP)
		return -3;

	return 0;

}

/************************************************************************
 ** 函数名称: MQTTSubscribe
 ** 函数功能: 订阅消息
 ** 入口参数: int32_t sock：套接字
 **           int8_t *topic：主题
 **           enum QoS pos：消息质量
 ** 出口参数: >=0:发送成功 <0:发送失败
 ** 备    注:
 ************************************************************************/
int32_t MQTTSubscribe(int32_t sock, char *topic, enum QoS pos)
{
	static uint32_t PacketID = 0;

	EventBits_t eventValue;

	uint16_t packetidbk = 0;
	int32_t conutbk = 0;
	uint8_t buf[100];
	int32_t buflen = sizeof(buf);
	MQTTString topicString = MQTTString_initializer;
	int32_t len;
	int32_t req_qos,
	qosbk;

	topicString.cstring = (char *) topic;
	req_qos = pos;

	len = MQTTSerialize_subscribe(buf, buflen, 0, PacketID++, 1, &topicString, &req_qos);
	if (transport_sendPacketBuffer(buf, len) < 0)
		return -1;

	eventValue = xEventGroupWaitBits(wifiEventHandler, wifi_link_ready[WIFI_MQTT_LINK_ID], pdTRUE, pdFALSE,
			(WIFI_UART_WAITTIME * 10));
	if ((eventValue & wifi_link_ready[WIFI_MQTT_LINK_ID]) != 0)
	{
		first_index_to_read = 0;
		if (MQTTPacket_read(buf, buflen, transport_getdata) != SUBACK)
			return -4;

		if (MQTTDeserialize_suback(&packetidbk, 1, &conutbk, &qosbk, buf, buflen) != 1)
			return -5;

		if ((qosbk == 0x80) || (packetidbk != (PacketID - 1)))
			return -6;

		return 0;
	}
	else
	{
		printf("WIFI UART get response overtime\n");
		return -2;
	}
}

int32_t MQTTUnSubscribe(int32_t sock, char *topic)
{
	static uint32_t PacketID = 0;

	EventBits_t eventValue;

	uint16_t packetidbk = 0;
	uint8_t buf[100];
	int32_t buflen = sizeof(buf);
	MQTTString topicString = MQTTString_initializer;
	int32_t len;

	topicString.cstring = (char *) topic;

	len = MQTTSerialize_unsubscribe(buf, buflen, 0, PacketID++, 1, &topicString);
	if (transport_sendPacketBuffer(buf, len) < 0)
		return -1;

	eventValue = xEventGroupWaitBits(wifiEventHandler, wifi_link_ready[WIFI_MQTT_LINK_ID], pdTRUE, pdFALSE,
			(WIFI_UART_WAITTIME * 10));
	if ((eventValue & wifi_link_ready[WIFI_MQTT_LINK_ID]) != 0)
	{
		first_index_to_read = 0;
		if (MQTTPacket_read(buf, buflen, transport_getdata) != SUBACK)
			return -4;

		if (MQTTDeserialize_unsuback(&packetidbk, buf, buflen) != 1)
			return -5;

		return 0;
	}
	else
	{
		printf("WIFI UART get response overtime\n");
		return -2;
	}
}



/************************************************************************
 ** 函数名称: UserMsgCtl
 ** 函数功能: 用户消息处理函数
 ** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
 ** 出口参数: 无
 ** 备    注:
 ************************************************************************/
void UserMsgCtl(MQTT_USER_MSG *msg)
{
	switch (msg->msgqos)
	{
	case 0:
//		PRINT_DEBUG("MQTT>>��Ϣ������QoS0\n");
		break;
	case 1:
//		PRINT_DEBUG("MQTT>>��Ϣ������QoS1\n");
		break;
	case 2:
//		PRINT_DEBUG("MQTT>>��Ϣ������QoS2\n");
		break;
	default:
//		PRINT_DEBUG("MQTT>>�������Ϣ����\n");
		break;
	}
//	PRINT_DEBUG("MQTT>>��Ϣ���⣺%s\n", msg->topic);
//	PRINT_DEBUG("MQTT>>��Ϣ���ݣ�%s\n", msg->msg);
//	PRINT_DEBUG("MQTT>>��Ϣ���ȣ�%d\n", msg->msglenth);
	Proscess(msg->msg);

	msg->valid = 0;
}

/************************************************************************
 ** 函数名称: GetNextPackID
 ** 函数功能: 产生下一个数据包ID
 ** 入口参数: 无
 ** 出口参数: uint16_t packetid:产生的ID
 ** 备    注:
 ************************************************************************/
uint16_t GetNextPackID(void)
{
	static uint16_t pubpacketid = 0;
	return pubpacketid++;
}

/************************************************************************
 ** 函数名称: mqtt_msg_publish
 ** 函数功能: 用户推送消息
 ** 入口参数: MQTT_USER_MSG  *msg：消息结构体指针
 ** 出口参数: >=0:发送成功 <0:发送失败
 ** 备    注:
 ************************************************************************/
int32_t MQTTMsgPublish(int32_t sock, char *topic, int8_t qos, uint8_t* msg)
{
	int8_t retained = 0;
	uint32_t msg_len;
	uint8_t buf[MSG_MAX_LEN];
	int32_t buflen = sizeof(buf), len = 0;
	MQTTString topicString = MQTTString_initializer;
	uint16_t packid = 0,
	packetidbk;

	topicString.cstring = (char *) topic;

	if ((qos == QOS1) || (qos == QOS2))
	{
		packid = GetNextPackID();
	}
	else
	{
		qos = QOS0;
		retained = 0;
		packid = 0;
	}

	msg_len = strlen((char *) msg);

	//send package
	len = MQTTSerialize_publish(buf, buflen, 0, qos, retained, packid, topicString, (unsigned char*) msg, msg_len);
	if (len <= 0)
		return -1;
	if (transport_sendPacketBuffer(buf, len) < 0)
		return -2;

	//QOS0 no return;
	if (qos == QOS0)
	{
		return 0;
	}
	else if (qos == QOS1)
	{
		if (WaitForPacket(sock, PUBACK, 5) < 0)
			return -3;
		return 1;
	}
	else if (qos == QOS2)
	{
		//wait PUBREC
		if (WaitForPacket(sock, PUBREC, 5) < 0)
			return -3;
		//send PUBREL
		len = MQTTSerialize_pubrel(buf, buflen, 0, packetidbk);
		if (len == 0)
			return -4;
		if (transport_sendPacketBuffer(buf, len) < 0)
			return -6;
		//wait PUBCOMP
		if (WaitForPacket(sock, PUBREC, 5) < 0)
			return -7;
		return 2;
	}
	//QOS error
	return -8;
}

/************************************************************************
 ** 函数名称: ReadPacketTimeout
 ** 函数功能: 阻塞读取MQTT数据
 ** 入口参数: int32_t sock:网络描述符
 **           uint8_t *buf:数据缓存区
 **           int32_t buflen:缓冲区大小
 **           uint32_t timeout:超时时间--0-表示直接查询，没有数据立即返回
 ** 出口参数: -1：错误,其他--包类型
 ** 备    注:
 ************************************************************************/
int32_t ReadPacketTimeout(int32_t sock, uint8_t *buf, int32_t buflen, uint32_t timeout)
{
#if 0
	fd_set readfd;
	struct timeval tv;

	if (timeout != 0)
	{
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		FD_ZERO(&readfd);
		FD_SET(sock, &readfd);

		if (select(sock + 1, &readfd, NULL, NULL, &tv) == 0)
		return -1;
		if (FD_ISSET(sock,&readfd) == 0)
		return -1;
	}
#endif
	first_index_to_read = 0;
	return MQTTPacket_read(buf, buflen, transport_getdata);
}

/************************************************************************
 ** 函数名称: deliverMessage
 ** 函数功能: 接受服务器发来的消息
 ** 入口参数: MQTTMessage *msg:MQTT消息结构体
 **           MQTT_USER_MSG *mqtt_user_msg:用户接受结构体
 **           MQTTString  *TopicName:主题
 ** 出口参数: 无
 ** 备    注:
 ************************************************************************/
void deliverMessage(MQTTString *TopicName, MQTTMessage *msg, MQTT_USER_MSG *mqtt_user_msg)
{
	mqtt_user_msg->msgqos = msg->qos;
	memcpy(mqtt_user_msg->msg, msg->payload, msg->payloadlen);
	mqtt_user_msg->msg[msg->payloadlen] = 0;
	mqtt_user_msg->msglenth = msg->payloadlen;
	memcpy((char *) mqtt_user_msg->topic, TopicName->lenstring.data, TopicName->lenstring.len);
	mqtt_user_msg->topic[TopicName->lenstring.len] = 0;
	mqtt_user_msg->packetid = msg->id;
	mqtt_user_msg->valid = 1;
}

/************************************************************************
 ** 函数名称: mqtt_pktype_ctl
 ** 函数功能: 根据包类型进行处理
 ** 入口参数: uint8_t packtype:包类型
 ** 出口参数: 无
 ** 备    注:
 ************************************************************************/
void mqtt_pktype_ctl(uint8_t packtype, uint8_t *buf, uint32_t buflen)
{
	MQTTMessage msg;
	int32_t rc;
	MQTTString receivedTopic;
	uint32_t len;
	switch (packtype)
	{
	case PUBLISH:
		if (MQTTDeserialize_publish(&msg.dup, (int*) &msg.qos, &msg.retained, &msg.id, &receivedTopic,
				(unsigned char **) &msg.payload, &msg.payloadlen, buf, buflen) != 1)
			return;
		deliverMessage(&receivedTopic, &msg, &mqtt_user_msg);

		if (msg.qos == QOS0)
		{
			UserMsgCtl(&mqtt_user_msg);
			return;
		}
		if (msg.qos == QOS1)
		{
			len = MQTTSerialize_puback(buf, buflen, mqtt_user_msg.packetid);
			if (len == 0)
				return;
			if (transport_sendPacketBuffer(buf, len) < 0)
				return;
			UserMsgCtl(&mqtt_user_msg);
			return;
		}

		if (msg.qos == QOS2)
		{
			len = MQTTSerialize_ack(buf, buflen, PUBREC, 0, mqtt_user_msg.packetid);
			if (len == 0)
				return;
			transport_sendPacketBuffer(buf, len);
		}
		break;
	case PUBREL:
		rc = MQTTDeserialize_ack(&msg.type, &msg.dup, &msg.id, buf, buflen);
		if ((rc != 1) || (msg.type != PUBREL) || (msg.id != mqtt_user_msg.packetid))
			return;
		if (mqtt_user_msg.valid == 1)
		{
			UserMsgCtl(&mqtt_user_msg);
		}
		len = MQTTSerialize_pubcomp(buf, buflen, msg.id);
		if (len == 0)
			return;
		transport_sendPacketBuffer(buf, len);
		break;
	case PUBACK:
		break;
	case PUBREC:
		break;
	case PUBCOMP:
		break;
	default:
		break;
	}
}

/************************************************************************
 ** 函数名称: WaitForPacket
 ** 函数功能: 等待特定的数据包
 ** 入口参数: int32_t sock:网络描述符
 **           uint8_t packettype:包类型
 **           uint8_t times:等待次数
 ** 出口参数: >=0:等到了特定的包 <0:没有等到特定的包
 ** 备    注:
 ************************************************************************/
int32_t WaitForPacket(int32_t sock, uint8_t packettype, uint8_t times)
{
	int32_t type;
	uint8_t buf[MSG_MAX_LEN];
	uint8_t n = 0;
	int32_t buflen = sizeof(buf);
	do
	{
		//��ȡ���ݰ�
		type = ReadPacketTimeout(sock, buf, buflen, 2);
		if (type != -1)
			mqtt_pktype_ctl(type, buf, buflen);
		n++;
	} while ((type != packettype) && (n < times));
	//�յ������İ�
	if (type == packettype)
		return 0;
	else
		return -1;
}

void Client_Connect(void)
{
	char* host_ip = HOST_NAME;
	MQTT_START:

	Debug("start to connect to aliyun %s, port %d\n", host_ip, HOST_PORT);
	MQTT_Socket = transport_open((int8_t*) host_ip, HOST_PORT);

	if (MQTT_Connect() != Connect_OK)
	{
		Error("auth failed, reconnect\n");
		transport_close(MQTT_Socket);
		vTaskDelay(DELAY_BASE_SEC_TIME * 5);
		goto MQTT_START;
	}

	//MQTTUnSubscribe(MQTT_Socket, (char *) TOPIC);


#if 0
	if (MQTTSubscribe(MQTT_Socket, (char *) TOPIC_SUB, QOS1) < 0)
	{
		Error("sub failed, reconnect\n");
		transport_close(MQTT_Socket);
		vTaskDelay(DELAY_BASE_SEC_TIME * 5);
		goto MQTT_START;
	}
#endif
	Debug("connect to Aliyun \n");
	xEventGroupSetBits(wifiEventHandler, wifi_link[WIFI_MQTT_LINK_ID]);

}

void AutoReloadCallback(void const * argument)
{
	xTaskNotifyGive(KEEP_Handle);
}

/************************************************************************
 ** 函数名称: mqtt_recv_thread
 ** 函数功能: MQTT任务
 ** 入口参数: void *pvParameters：任务参数
 ** 出口参数: 无
 ** 备    注: MQTT连云步骤：
 **           1.连接对应云平台的服务器
 **           2.MQTT用户与秘钥验证登陆
 **           3.订阅指定主题
 **           4.等待接收主题的数据与上报主题数据
 ************************************************************************/
void mqtt_recv_thread(void *pvParameters)
{
	uint8_t buf[MSG_MAX_LEN];
	int32_t buflen = sizeof(buf);
	int32_t type;

	BaseType_t err = pdFALSE;
	EventBits_t eventValue = 0;

	while (1)
	{
		eventValue = xEventGroupWaitBits(wifiEventHandler,
				(wifi_link_ready[WIFI_MQTT_LINK_ID] | wifi_link[WIFI_MQTT_LINK_ID]),
				pdFALSE,
		pdTRUE, portMAX_DELAY);

		if ((eventValue & (wifi_link_ready[WIFI_MQTT_LINK_ID] | wifi_link[WIFI_MQTT_LINK_ID])) == 0)
		{
			printf("wait mqtt info rec\n");
			continue;
		}
		xEventGroupClearBits(wifiEventHandler, wifi_link_ready[WIFI_MQTT_LINK_ID]);
		err = xSemaphoreTake(wifiSemaphoreMutexHandle, (WIFI_UART_WAITTIME * 4));
		if (err == pdFALSE)
		{
			Debug("get wifiSemaphoreMutexHandle failed, wait next time\n");
			continue;
		}

		type = ReadPacketTimeout(MQTT_Socket, buf, buflen, 0);
		if (type != -1)
		{
			xTimerReset(AR_Mqtt_HB_TimerHandle, 10);
			mqtt_pktype_ctl(type, buf, buflen);
		}
		xSemaphoreGive(wifiSemaphoreMutexHandle);
	}
}

void mqtt_send_thread(void *pvParameters)
{
	uint8_t res;
	int32_t ret = 0;
	BaseType_t err = pdFALSE;

	//BaseType_t xReturn = pdTRUE;
	//DHT11_Data_TypeDef* recv_data;
	cJSON* cJSON_Data = NULL;
	cJSON_Data = cJSON_Data_Init();
	double a, b;

	while (1)
	{

		//TODO 完善发送任务

		xEventGroupWaitBits(wifiEventHandler, wifi_link[WIFI_MQTT_LINK_ID], pdFALSE, pdFALSE, portMAX_DELAY);
		//xReturn = xQueueReceive(MQTT_Data_Queue, &recv_data, 1000);

		vTaskDelay(3000);
		//if (xReturn != pdTRUE)
		{
			err = xSemaphoreTake(wifiSemaphoreMutexHandle, (WIFI_UART_WAITTIME * 4));
			if (err == pdFALSE)
			{
				Debug("send task get wifiSemaphoreMutexHandle failed, wait next time\n ");
				continue;
			}
			Debug("send task get the wifi sema\n");
//			a = recv_data->temperature;
//			b = recv_data->humidity;
			a = 31.1;
			b = 52.2;
			res = cJSON_Update(cJSON_Data, TEMP_NUM, &a);
			res = cJSON_Update(cJSON_Data, HUM_NUM, &b);
			if (UPDATE_SUCCESS != res)
			{
				Debug("update JSON failed\n");
				continue;
			}

//			Debug("get Json string\n");
			char* p = cJSON_Print(cJSON_Data);
			Debug("start to send Json\n");
//			Debug("start to send Json: %s\n", p);
			ret = MQTTMsgPublish(MQTT_Socket, (char*) TOPIC, QOS0, (uint8_t*) p);
			xSemaphoreGive(wifiSemaphoreMutexHandle);

			Debug("send task give the wifi sema\n");

			if (ret >= 0)
			{
				xTimerReset(AR_Mqtt_HB_TimerHandle, 0);
				Debug("send task send msg success!\n");
			}
			else
			{
				Error("send mqtt msg failed!\n");
				xTaskNotifyGive(KEEP_Handle);
			}
			vPortFree(p);
			p = NULL;

		}
	}

}

void mqtt_keepAlive_thread(void *pvParameters)
{

	EventBits_t eventValue;
	int32_t pingReq = 0;
	BaseType_t err = pdFALSE;

	MQTT_APPLY_RAM:

	wifiLinkdata.data_buf[WIFI_MQTT_LINK_ID] = pvPortMalloc(WIFI_RX_BUF_SIZE);
	if (wifiLinkdata.data_buf[WIFI_MQTT_LINK_ID] == NULL)
	{
		Error("%s apply ram for MQTT data buffer failed!\n", pcTaskGetName(xTaskGetCurrentTaskHandle()));
		vTaskDelay(DELAY_BASE_SEC_TIME * 5);
		goto MQTT_APPLY_RAM;
	}

	wifiLinkdata.maxLenth[WIFI_MQTT_LINK_ID] = WIFI_RX_BUF_SIZE;

	MQTT_START:
	//TODO 获取模组当前状态，是否连入wifi，方式，查询ip

	xEventGroupWaitBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP, pdFALSE, pdFALSE, portMAX_DELAY);

	xSemaphoreTake(wifiSemaphoreMutexHandle, portMAX_DELAY);

	if (DIS_CON_AP == get_wifi_status())
	{
		xEventGroupClearBits(wifiEventHandler, EVENTBIT_WIFI_CONNECTED_AP);
		xEventGroupSetBits(wifiEventHandler, EVENTBIT_WIFI_DIS_ING_AP);
		xSemaphoreGive(wifiSemaphoreMutexHandle);

		goto MQTT_START;
	}

	Client_Connect();
	xSemaphoreGive(wifiSemaphoreMutexHandle);

	xEventGroupSetBits(wifiEventHandler, wifi_link[WIFI_MQTT_LINK_ID]);

	xTimerStart(AR_Mqtt_HB_TimerHandle, 0);
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		eventValue = xEventGroupGetBits(wifiEventHandler);
		if ((eventValue & wifi_link[WIFI_MQTT_LINK_ID]) == 0)
		{
			printf("MQTT disconnect, go to close the link first\n");
			goto MQTT_CLOSE;
		}
		else
		{
			err = xSemaphoreTake(wifiSemaphoreMutexHandle, (WIFI_UART_WAITTIME * 4));
			if (err == pdFALSE)
			{
				Debug("get wifiSemaphoreMutexHandle failed, wait next notify\n");
				continue;
			}

			get_wifi_status();

			//suspend rec task, so the task will not get the EVENTBIT_WIFI_UART_REC event
			vTaskSuspend(RECV_Handle);
			pingReq = MQTT_PingReq(WIFI_MQTT_LINK_ID);
			vTaskResume(RECV_Handle);

			if (pingReq < 0)
			{
				printf("send heartbeat failed, go to close the link first\n");
				xEventGroupClearBits(wifiEventHandler, wifi_link[WIFI_MQTT_LINK_ID]);
				goto MQTT_CLOSE;
			}
			xSemaphoreGive(wifiSemaphoreMutexHandle);
			Debug("send heartbeat suc\n");
		}
	}

	MQTT_CLOSE:

	transport_close(WIFI_MQTT_LINK_ID);
	xSemaphoreGive(wifiSemaphoreMutexHandle);
	goto MQTT_START;
}

void mqtt_thread_init(void)
{
	AR_Mqtt_HB_TimerHandle = xTimerCreate("mqtt_heartbeat_timer", HEARTBEAT_TIMER, pdTRUE, (void *) 1,
			AutoReloadCallback);

	wifiSemaphoreMutexHandle = xSemaphoreCreateMutex();



	xTaskCreate(mqtt_recv_thread, "mqtt_recv_thread", (configMINIMAL_STACK_SIZE * 8), NULL, (tskIDLE_PRIORITY + 3),
			&RECV_Handle);

	xTaskCreate(mqtt_keepAlive_thread, "mqtt_keepAlive_thread", (configMINIMAL_STACK_SIZE * 8), NULL,
			(tskIDLE_PRIORITY + 5), &KEEP_Handle);

	xTaskCreate(mqtt_send_thread, "mqtt_send_thread", (configMINIMAL_STACK_SIZE * 8), NULL, (tskIDLE_PRIORITY + 4),
			&SEND_Handle);
	if (AR_Mqtt_HB_TimerHandle == NULL || wifiSemaphoreMutexHandle == NULL)
	{
		printf("create timer or wifi semaphore mutex failed!\n");
	}

}

