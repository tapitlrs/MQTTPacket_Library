/* --COPYRIGHT--,
 * Copyright (c)2019, TAPIT Co.,Ltd.
 * https://tapit.vn
 *
 **************************_TAPIT_MQTTControlPacket_******************
 *  Description:	Use to make the MQTT Control Packets	
 *  Version:  		1.1
 *  Author: 		Thang Dau
 *  Release: 		October 27, 2019
 *  Built with CubeMX Version 5.3.0 and Keil C Version 5.26.2
 *********************************************************************
 */
 
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "stdio.h"

#define sim_huart	huart3 

#define MQTT_VERSION_3_1      3
#define MQTT_VERSION_3_1_1    4

// MQTT_VERSION : Pick the version
#define MQTT_VERSION MQTT_VERSION_3_1
#ifndef MQTT_VERSION
#define MQTT_VERSION MQTT_VERSION_3_1_1
#endif

#ifndef MQTT_MAX_PACKET_SIZE
#define MQTT_MAX_PACKET_SIZE 200UL
#endif

#ifndef MQTT_MAX_PAYLOAD_SIZE
#define MQTT_MAX_PAYLOAD_SIZE 160UL
#endif

#ifndef MQTT_MAX_TOPIC_SIZE
#define MQTT_MAX_TOPIC_SIZE MQTT_MAX_PACKET_SIZE - MQTT_MAX_PAYLOAD_SIZE
#endif


#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 600
#endif

#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 6
#endif

#define MQTT_CONNECTION_TIMEOUT     -4
#define MQTT_CONNECTION_LOST        -3
#define MQTT_CONNECT_FAILED         -2
#define MQTT_DISCONNECTED           -1
#define MQTT_CONNECTED               0
#define MQTT_CONNECT_BAD_PROTOCOL    1
#define MQTT_CONNECT_BAD_CLIENT_ID   2
#define MQTT_CONNECT_UNAVAILABLE     3
#define MQTT_CONNECT_BAD_CREDENTIALS 4
#define MQTT_CONNECT_UNAUTHORIZED    5

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MQTTQOS0        (0 << 1)
#define MQTTQOS1        (1 << 1)
#define MQTTQOS2        (2 << 1)

#define START_RL_LOC	1
#define TOPICNAME_BYTES  2

#define TRYSENDMQTT_MAX 3		//Try send time 

// Struct for Connect packet
typedef struct 
{
	char* ClientID;
	bool CleanSession;
	bool usernameFlag;
	char* UserName; 
	bool passwordFlag;
	char* Password;
	bool willFlag;
	char* willTopic;
	uint8_t willQoS;
	bool willRetain;
	char* willMessage;
}mqtt_connect_t;

// Struct for Connack packet
typedef struct 
{
	bool session_present;
	uint8_t connect_return;
}mqtt_connack_t;

// Struct for Publish packet
typedef struct 
{
	uint8_t Qos;
	bool retain;
	bool DUP_Flag;
	char topic[MQTT_MAX_TOPIC_SIZE ]; //MQTT_MAX_TOPIC ~ MQTT_MAX_PACKET_SIZE - MQTT_MAX_PAYLOAD_SIZE - 9
	char payload[MQTT_MAX_PAYLOAD_SIZE];
	uint16_t topic_size;
	uint16_t payload_size;
	uint16_t packetID;
}mqtt_publish_t;

// Struct for Puback packet
typedef struct 
{
	uint16_t packetID;
}mqtt_puback_t;

// Struct for Pubrec packet
typedef struct 
{
	uint16_t packetID;
}mqtt_pubrec_t;

// Struct for Pubrel packet
typedef struct 
{
	uint16_t packetID;
}mqtt_pubrel_t;

// Struct for Pubcomp packet
typedef struct 
{
	uint16_t packetID;
}mqtt_pubcomp_t;

// Struct for Subscribe packet
typedef struct 
{
	uint16_t packetID;
	uint16_t topic_size;
	char topic[MQTT_MAX_TOPIC_SIZE];
	uint8_t QoS;
}mqtt_subscribe_t;

// Struct for Suback packet
typedef struct 
{
	uint16_t packetID;
	uint8_t return_code;
}mqtt_suback_t;

// Struct for Unsubscribe packet
typedef struct
{	
	uint16_t packetID;
	uint16_t topic_size;
	char topic[MQTT_MAX_TOPIC_SIZE];
}mqtt_unsubscribe_t;

// Struct for Unsuback packet
typedef struct 
{
	uint16_t packetID;
	uint8_t return_code;
}mqtt_unsuback_t;


typedef struct
{
	uint8_t MQTT_PacketType;
	union
	{
		mqtt_connack_t  mqtt_connack;
		mqtt_publish_t  mqtt_publish;
		mqtt_puback_t   mqtt_puback;
		mqtt_pubrec_t 	mqtt_pubrec;
		mqtt_pubrel_t		mqtt_pubrel;
		mqtt_pubcomp_t  mqtt_pubcomp;
		mqtt_suback_t   mqtt_suback;
		mqtt_unsuback_t mqtt_unsuback;
	}packet;
}mqtt_packetRcv_t;

/**
  * @brief  Decode arrays into the package structure
  * @param  
	*					QosPacket: 		the pointer of Struct that contains the result after analyzing
	*					PacketArray:	the array contains MQTT packet
	*					lenthPkg:			length of PacketArray
  * @retval true:   the MQTT Control Packet type is true
  *					false:  connect failed
  */
bool 		 MQTT_decode(mqtt_packetRcv_t* QosPacket, uint8_t* PacketArray, uint64_t lengthPkg);	

/**
  * @brief  Create the MQTT connect packet 
  * @param  
	*					PacketArray:	the array contains MQTT packet
	*					mqtt_connect:	the structure contains the connection parameters 
  * @retval length of Connect Packet in PacketArray
  */
uint16_t MQTT_ConnectPacket(uint8_t * PacketArray,mqtt_connect_t mqtt_connect);

/**
  * @brief  Create the MQTT publish packet 
  * @param  
	*					PacketArray:	the array contains MQTT packet
	*					mqtt_publish:	the structure contains the publish parameters 
  * @retval length of Connect Packet in PacketArray
  */
uint16_t MQTT_PublishPacket (uint8_t * PacketArray, mqtt_publish_t mqtt_publish);
		
/**
  * @brief  Create the MQTT Publish message packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Connect packet
	*					mqtt_puback:	the structure contains the Publish message
  * @retval length of Publish message Packet in PacketArray
  */		
uint16_t MQTT_PubBackPacket (uint8_t * PacketArray, mqtt_puback_t mqtt_puback);

/**
  * @brief  Create the MQTT Publish acknowledgement packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Publish acknowledgement packet
	*					mqtt_pubrec:	the structure contains the Publish acknowledgement parameters 
  * @retval length of Publish acknowledgement Packet in PacketArray
  */	
uint16_t MQTT_PubRecPacket (uint8_t * PacketArray, mqtt_pubrec_t mqtt_pubrec);

/**
  * @brief  Create the MQTT Publish release packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Publish release packet
	*					mqtt_pubrel:	the structure contains the Publish release parameters 
  * @retval length of Publish release Packet in PacketArray
  */	
uint16_t MQTT_PubRelPacket (uint8_t * PacketArray, mqtt_pubrel_t mqtt_pubrel);

/**
  * @brief  Create the MQTT Publish complete packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Publish complete packet
	*					mqtt_pubcomp:	the structure contains the Publish complete parameters 
  * @retval length of Publish complete Packet in PacketArray
  */	
uint16_t MQTT_PubCompPacket (uint8_t * PacketArray, mqtt_pubcomp_t mqtt_pubcomp);

/**
  * @brief  Create the MQTT Subscribe packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Subscribe packet
	*					mqtt_subscribe:	the structure contains the Subscribe parameters 
  * @retval length of Subscribe Packet in PacketArray
  */	
uint16_t MQTT_SubscribePacket (uint8_t * PacketArray, mqtt_subscribe_t mqtt_subscribe);

/**
  * @brief  Create the MQTT Unsubscribe packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Unsubscribe packet
	*					mqtt_unsubscribe:	the structure contains the Unsubscribe parameters 
  * @retval length of Unsubscribe Packet in PacketArray
  */	
uint16_t MQTT_UnsubscribePacket (uint8_t * PacketArray, mqtt_unsubscribe_t mqtt_unsubscribe);

/**
  * @brief  Create the MQTT Ping request packet 
  * @param  
	*					PacketArray:	pointer of the array contains MQTT Unsubscribe packet
	*					mqtt_unsubscribe:	the structure contains the Unsubscribe parameters 
  * @retval length of Unsubscribe Packet in PacketArray
  */	
uint16_t MQTT_PingReqPacket (uint8_t * PacketArray);


