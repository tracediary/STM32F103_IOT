#ifndef __MALLOC_H
#define __MALLOC_H
#include "system.h"
//#include "lwipopts.h"

#define   MSG_MAX_LEN     500
#define   MSG_TOPIC_LEN   100
#define   KEEPLIVE_TIME   120
#define   MQTT_VERSION    4

#define		HEARTBEAT_TIMER				(KEEPLIVE_TIME * 1000)


#define   HOST_NAME       "a1NTyrvBAf4.iot-as-mqtt.cn-shanghai.aliyuncs.com"

#define   HOST_PORT     1883

#define   CLIENT_ID     "cca81|securemode=3,signmethod=hmacsha1|"
#define   USER_NAME     "stm32_temlate&a1NTyrvBAf4"
#define   PASSWORD      "C05DBFA9C25F4E4FA251674587C7ABE966DDF5C4"

#define   TOPIC         "/a1NTyrvBAf4/stm32_temlate/user/template"
#define   TEST_MESSAGE  "test_message"

enum QoS
{
	QOS0 = 0, QOS1, QOS2
};

enum MQTT_Connect
{
	Connect_OK = 0, Connect_NOK, Connect_NOTACK
};

//���ݽ����ṹ��
typedef struct __MQTTMessage
{
	uint32_t qos;
	uint8_t retained;
	uint8_t dup;
	uint16_t id;
	uint8_t type;
	void *payload;
	int32_t payloadlen;
} MQTTMessage;

//user rec struct
typedef struct __MQTT_MSG
{
	uint8_t msgqos;                 //消息质量
	uint8_t msg[MSG_MAX_LEN];       //消息内容
	uint32_t msglenth;              //消息长度
	uint8_t topic[MSG_TOPIC_LEN];   //主题
	uint16_t packetid;              //消息ID
	uint8_t valid;                  //消息是否有效
} MQTT_USER_MSG;

//user send struct
typedef struct
{
	int8_t topic[MSG_TOPIC_LEN];
	int8_t qos;
	int8_t retained;

	uint8_t msg[MSG_MAX_LEN];
	uint8_t msglen;
} mqtt_recv_msg_t, *p_mqtt_recv_msg_t, mqtt_send_msg_t, *p_mqtt_send_msg_t;

void mqtt_thread(void *pvParameters);

int32_t MQTT_PingReq(int32_t sock);

uint8_t MQTT_Connect(void);

int32_t MQTTSubscribe(int32_t sock, char *topic, enum QoS pos);

void UserMsgCtl(MQTT_USER_MSG *msg);

uint16_t GetNextPackID(void);

//int32_t MQTTMsgPublish(int32_t sock, char *topic, int8_t qos, int8_t retained,uint8_t* msg,uint32_t msg_len);
int32_t MQTTMsgPublish(int32_t sock, char *topic, int8_t qos, uint8_t* msg);

int32_t ReadPacketTimeout(int32_t sock, uint8_t *buf, int32_t buflen, uint32_t timeout);

void mqtt_pktype_ctl(uint8_t packtype, uint8_t *buf, uint32_t buflen);

int32_t WaitForPacket(int32_t sock, uint8_t packettype, uint8_t times);

void mqtt_thread_init(void);

#endif

