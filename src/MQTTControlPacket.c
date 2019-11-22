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
#include "MQTTControlPacket.h"

static uint8_t MQTT_FixedHeader(uint8_t header, uint8_t* buf, uint16_t length)
{
	uint8_t lenBuf[4];
	uint8_t llen = 0;
	uint8_t digit;
	uint8_t pos = 0;
	uint16_t len = length;
	do {
			digit = len % 128;
			len = len / 128;
			if (len > 0) {
					digit |= 0x80;
			}
			lenBuf[pos++] = digit;
			llen++;
	} while(len>0);

	buf[0] = header;
	for (int i=0;i<llen;i++) 
	{
			buf[1+i] = lenBuf[i];
	}
	return llen+1;
}

static uint16_t writeString(const char* string, uint8_t* buf, uint16_t pos) {
    const char* idp = string;
    uint16_t i = 0;
    pos += 2;
    while (*idp) {
        buf[pos++] = *idp++;
        i++;
    }
    buf[pos-i-2] = (i >> 8);
    buf[pos-i-1] = (i & 0xFF);
    return pos;
}


uint16_t MQTT_ConnectPacket( uint8_t * PacketArray,mqtt_connect_t mqtt_connect)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint8_t length =0;
#if MQTT_VERSION == MQTT_VERSION_3_1
    uint8_t d[9] = {0x00,0x06,'M','Q','I','s','d','p', MQTT_VERSION};
		#define MQTT_HEADER_VERSION_LENGTH 9
#elif MQTT_VERSION == MQTT_VERSION_3_1_1
    uint8_t d[7] = {0x00,0x04,'M','Q','T','T',MQTT_VERSION};
		#define MQTT_HEADER_VERSION_LENGTH 7
#endif
	for (int j = 0;j<MQTT_HEADER_VERSION_LENGTH;j++) 
	{
		buffer[length++] = d[j];
	}
	uint8_t v;
	if (mqtt_connect.willFlag == 1) 
	{
		v = 0x04|(mqtt_connect.willQoS<<3)|(mqtt_connect.willRetain<<5)|(mqtt_connect.CleanSession<<1);
	} 
	else 
	{
		v = (mqtt_connect.CleanSession<<1);
	}
	if(mqtt_connect.usernameFlag == 1) 
	{
		v = v|0x80;
		if(mqtt_connect.passwordFlag == 1) 
		{
			 v = v|(0x80>>1);
		}
	}
	buffer[length++] = v;
	buffer[length++] = ((MQTT_KEEPALIVE) >> 8);
	buffer[length++] = ((MQTT_KEEPALIVE) & 0xFF);
	length = writeString(mqtt_connect.ClientID,buffer,length);
	if (mqtt_connect.willFlag == 1 ) 
	{
		 length = writeString(mqtt_connect.willTopic,buffer,length);
		 length = writeString(mqtt_connect.willMessage,buffer,length);
	}
	if(mqtt_connect.usernameFlag == 1) 
	{
		length = writeString(mqtt_connect.UserName,buffer,length);
		if(mqtt_connect.usernameFlag == 1) 
		{
			length = writeString(mqtt_connect.Password,buffer,length);
		}
	}
	
	uint16_t fixedHeader_length = MQTT_FixedHeader(MQTTCONNECT,PacketArray,length);
	uint16_t packet_length;
	for(packet_length = fixedHeader_length; packet_length < length + fixedHeader_length;packet_length++)
	{
		PacketArray[packet_length] = buffer[packet_length-fixedHeader_length];
	}
	return packet_length;
}

uint16_t MQTT_PublishPacket (uint8_t * PacketArray, mqtt_publish_t mqtt_publish)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint16_t length=0;
	length = writeString(mqtt_publish.topic,buffer,length);
	if(mqtt_publish.Qos != 0)
	{
		buffer[length++] = (mqtt_publish.packetID >> 8);
		buffer[length++] = (mqtt_publish.packetID & 0xFF);
	}
	uint16_t i;
	for (i=0;i<mqtt_publish.payload_size;i++) 
	{
     buffer[length++] = mqtt_publish.payload[i];
  }
	uint8_t header = MQTTPUBLISH;
	if(mqtt_publish.Qos != 0)
	{
		header |= (mqtt_publish.Qos<<1);
		header |= mqtt_publish.DUP_Flag<<3;
	}
	if (mqtt_publish.retain) header |= 1;
	uint16_t fixedHeader_length = MQTT_FixedHeader(header,PacketArray,length);
	uint16_t packet_length;
	for(packet_length = fixedHeader_length; packet_length < length + fixedHeader_length;packet_length++)
	{
		PacketArray[packet_length] = buffer[packet_length-fixedHeader_length];
	}
	return packet_length;
}

uint16_t MQTT_PubBackPacket (uint8_t * PacketArray, mqtt_puback_t mqtt_puback)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	PacketArray[0] = 0x40;
	PacketArray[1] = 0x02;
	PacketArray[2] = (uint8_t)(mqtt_puback.packetID >> 8);
	PacketArray[3] = (uint8_t)(mqtt_puback.packetID & 0xFF);
	return 4;
}	

uint16_t MQTT_PubRecPacket (uint8_t * PacketArray, mqtt_pubrec_t mqtt_pubrec)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	PacketArray[0] = 0x50;
	PacketArray[1] = 0x02;
	PacketArray[2] = (uint8_t)(mqtt_pubrec.packetID >> 8);
	PacketArray[3] = (uint8_t)(mqtt_pubrec.packetID & 0xFF);
	return 4;
}	

uint16_t MQTT_PubRelPacket (uint8_t * PacketArray, mqtt_pubrel_t mqtt_pubrel)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	PacketArray[0] = 0x62;
	PacketArray[1] = 0x02;
	PacketArray[2] = (uint8_t)(mqtt_pubrel.packetID >> 8);
	PacketArray[3] = (uint8_t)(mqtt_pubrel.packetID & 0xFF);
	return 4;
}	

uint16_t MQTT_PubCompPacket (uint8_t * PacketArray, mqtt_pubcomp_t mqtt_pubcomp)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	PacketArray[0] = 0x70;
	PacketArray[1] = 0x02;
	PacketArray[2] = (uint8_t)(mqtt_pubcomp.packetID >> 8);
	PacketArray[3] = (uint8_t)(mqtt_pubcomp.packetID & 0xFF);
	return 4;
}	

uint16_t MQTT_SubscribePacket (uint8_t * PacketArray, mqtt_subscribe_t mqtt_subscribe)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint16_t length=0;
	buffer[length++] = (mqtt_subscribe.packetID >> 8);
	buffer[length++] = (mqtt_subscribe.packetID & 0xFF);
	length = writeString(mqtt_subscribe.topic,buffer,length);
	buffer[length++] = mqtt_subscribe.QoS;
	
	uint16_t fixedHeader_length = MQTT_FixedHeader(MQTTSUBSCRIBE | 0x02,PacketArray,length);
	uint16_t packet_length;
	for(packet_length = fixedHeader_length; packet_length < length + fixedHeader_length;packet_length++)
	{
		PacketArray[packet_length] = buffer[packet_length-fixedHeader_length];
	}
	return packet_length;
}	

uint16_t MQTT_UnsubscribePacket (uint8_t * PacketArray, mqtt_unsubscribe_t mqtt_unsubscribe)
{
	uint16_t packet_length;
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	uint8_t buffer[MQTT_MAX_PACKET_SIZE];
	uint16_t length=0;
	buffer[length++] = (mqtt_unsubscribe.packetID >> 8);
	buffer[length++] = (mqtt_unsubscribe.packetID & 0xFF);
	length = writeString(mqtt_unsubscribe.topic,buffer,length);
	
	uint16_t fixedHeader_length = MQTT_FixedHeader(MQTTUNSUBSCRIBE | 0x02,PacketArray,length);
	
	for(packet_length = fixedHeader_length; packet_length < length + fixedHeader_length;packet_length++)
	{
		PacketArray[packet_length] = buffer[packet_length-fixedHeader_length];
	}
	return packet_length;
}	

uint16_t MQTT_PingReqPacket (uint8_t * PacketArray)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	PacketArray[0] = 0xC0;
	PacketArray[1] = 0x00;
	return 2;
}	

uint16_t MQTT_DisconnnectPacket (uint8_t * PacketArray)
{
	for(int i = 0; i < MQTT_MAX_PACKET_SIZE; i++)
	{
		PacketArray[i] = 0;
	}
	PacketArray[0] = 0xE0;
	PacketArray[1] = 0x00;
	return 2;
}	

uint64_t decode_RemainingLengthPacket(uint8_t* pk_array)
{
	uint8_t ind = 1;
	uint64_t muliplier = 1;
	uint64_t value = 0;
	char encodedByte;
	do
	{
		encodedByte = pk_array[ind++]&0x00FF;
		value += ((uint64_t)encodedByte&0x7F)*muliplier;
		muliplier *=128;
		if(muliplier > 128*128*128)
		{
			printf("Error(Malformed Remaining Length)");
		}
	}while(((encodedByte & 0x80) != 0));
	return value;
}

bool MQTT_decode(mqtt_packetRcv_t* QosPacket, uint8_t* PacketArray, uint64_t lengthPkg)
{
	QosPacket->MQTT_PacketType = PacketArray[0]&0xF0;
	uint64_t lengthTopic;
	uint64_t RemainingLength = decode_RemainingLengthPacket(PacketArray);
	uint8_t Rlength_bytes=0; //Remaining length byte
	if(RemainingLength<=127) 							Rlength_bytes =1;
	else if(RemainingLength<=16383) 			Rlength_bytes =2;
	else if(RemainingLength<=2097151UL) 	Rlength_bytes =3;
	else if(RemainingLength<=268435455UL) Rlength_bytes =4;
	
	switch (QosPacket->MQTT_PacketType)
	{
		case MQTTCONNACK:
			QosPacket->packet.mqtt_connack.session_present 	= PacketArray[START_RL_LOC + Rlength_bytes];
			
			//	get the return code
			//	0x00 - Connection Accepted 
			//	0x01 - Connection Refused - unacceptable protocol version 
			//	0x02 - Connection Refused - identifier rejected
			//	0x03 - Connection Refused - Server unavailable
			// 	0x04 - Connection Refused - bad user name or password
			//	0x05 - Connection Refused - not authorized 
			QosPacket->packet.mqtt_connack.connect_return 	= PacketArray[START_RL_LOC + Rlength_bytes+1];
			break;
		
		case MQTTPUBLISH:
			QosPacket->packet.mqtt_publish.Qos = (PacketArray[0]>>1)&0x03;			//Qos flag
			QosPacket->packet.mqtt_publish.DUP_Flag = (PacketArray[0]>>3)&0x01;//Duplicate flag
			QosPacket->packet.mqtt_publish.retain = (PacketArray[0])&0x01;			//retain flag
			//Get length of Topic Name
			lengthTopic = ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
			//Get Topic Name
			for(int i=0;i<lengthTopic;i++)
			{
				QosPacket->packet.mqtt_publish.topic[i] = PacketArray[TOPICNAME_BYTES + START_RL_LOC + Rlength_bytes+i];
			}
			//if QoS != 0, get the packet Identifier
			if(QosPacket->packet.mqtt_publish.Qos != 0) //check QoS
			{
				QosPacket->packet.mqtt_publish.packetID = ((uint16_t)PacketArray[3+Rlength_bytes+lengthTopic]<<8)|(PacketArray[4+Rlength_bytes+lengthTopic]);
				QosPacket->packet.mqtt_publish.payload_size= lengthPkg-lengthTopic-5-Rlength_bytes;
				//get the message
				for(int i=lengthTopic+5+Rlength_bytes;i<lengthPkg;i++)
				{
					QosPacket->packet.mqtt_publish.payload[i-lengthTopic-5-Rlength_bytes] = PacketArray[i];
				}
			}
			else
			{
				//get the message
				for(int i=lengthTopic+3+Rlength_bytes;i<lengthPkg;i++)
				{
					QosPacket->packet.mqtt_publish.payload[i-lengthTopic-3-Rlength_bytes] = PacketArray[i];
				}
				QosPacket->packet.mqtt_publish.payload_size= lengthPkg-lengthTopic-3-Rlength_bytes;
			}
			
			break;
			
		case MQTTPUBACK:
			//get the packet identifier
			QosPacket->packet.mqtt_puback.packetID = ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
			break;
		
		case MQTTPUBREC:
			//get the packet identifier
			QosPacket->packet.mqtt_pubrec.packetID = ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
			break;
		
		case MQTTPUBREL:
			//get the packet identifier
			QosPacket->packet.mqtt_pubrel.packetID = ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
			break;
		
		case MQTTPUBCOMP:
			//get the packet identifier
			QosPacket->packet.mqtt_pubcomp.packetID = ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
			break;
		
		case MQTTSUBACK:
			//get the packet identifier
			QosPacket->packet.mqtt_suback.packetID 		= ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
			//	get the return code
			//	0x00 - Success - Maximum QoS 0 
			//	0x01 - Success - Maximum QoS 1 
			//	0x02 - Success - Maximum QoS 2 
			//	0x80 - Failure 
			QosPacket->packet.mqtt_suback.return_code = PacketArray[START_RL_LOC + (Rlength_bytes+2)];
			break;
		
		case MQTTPINGRESP:
			break;
		
		case MQTTUNSUBACK:
			//get the packet identifier
			QosPacket->packet.mqtt_unsuback.packetID 		= ((uint16_t)PacketArray[START_RL_LOC + Rlength_bytes]<<8)|PacketArray[START_RL_LOC + (Rlength_bytes+1)];
		
		default:
			return false;
	}
	return true;
}
