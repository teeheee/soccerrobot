/*
 * wifi.c
 *
 *  Created on: 02.06.2015
 *      Author: alexander
 */


#include "wifi.h"
#include "config.h"
#include "aktuators.h"
#include<string.h>

//initialisiert das xbee

XbeeWifi xbee;
char* remote_ip;
uint16_t remote_port;
uint8_t tcp_or_udp_flag;
volatile uint8_t modemstatus = 0;
volatile long last_message_time = 0;

#define nointerrupt detachInterrupt(XBEE_ATN)
#define interrupt attachInterrupt(XBEE_ATN, attn_interrupt, FALLING)

//#define AT_PRECONFIGURED

int get_last_message_time()
{
	return millis()-last_message_time;
}

void attn_interrupt() {
	unsigned int time = millis();
	do {
		xbee.process();
		if(millis()-time>10000)
		{
			ERROR_MESSAGE("[wifi] attn timed out!!!!\r\n");
			return;
		}
	}while (digitalRead(XBEE_ATN) == 0);
}



int write_AT_config() {
	int ret = 0;
	ret += xbee.at_cmd_noparm("AC");
	ret += xbee.at_cmd_noparm("WR");
	return ret;
}

void factory_reset() {
	Serial.println(F("[wifi] doing factory reset"));
	nointerrupt;
	xbee.at_cmd_noparm("RE");
	xbee.process();
	interrupt;
}

int atbyte(const char* AT, uint8_t b) {
	uint8_t querybuffer;
	int querylength;
	if (xbee.at_query(AT, &querybuffer, &querylength, 1))
		return 1;
	if (querylength != 1 || querybuffer != b)
		return xbee.at_cmd_byte(AT, b);
	return -1;
}

int atshort(const char* AT, uint16_t s) {
	uint16_t querybuffer;
	int querylength;
	if (xbee.at_query(AT, (uint8_t*) &querybuffer, &querylength, 2))
		return 1;
	if (querylength != 2 || querybuffer != s)
		return xbee.at_cmd_short(AT, s);
	return -1;
}

int atstring(const char* AT, const char* str) {
	uint8_t querybuffer[20];
	int querylength;
	if (xbee.at_query(AT, (uint8_t*) &querybuffer, &querylength, 2))
		return 1;
	if (querylength != (int) strlen(str)
			|| strcmp((const char*) querybuffer, str) != 0)
		return xbee.at_cmd_str(AT, str);
	return -1;
}



void inbound_ip(uint8_t * data, int size, s_rxinfo *info) //TODO CHECK CHECKSUM
{
	last_message_time = millis();
	befehl* b;
	if (size == sizeof(befehl)) {
		b = (befehl*) data;
		motors(b->motorlinks, b->motorrechts);
		if (b->kicker)
			kick();
		activate_camera(b->kamera);
	}
}

void inbound_status(uint8_t status) {
	modemstatus = status;
	Serial.print(F("[wifi] Modem status, received code "));
	Serial.print(status, DEC);
	Serial.print(F(" ("));

	switch (status) {
	case XBEE_MODEM_STATUS_RESET:
		Serial.print(F("Reset or power on"));
		break;
	case XBEE_MODEM_STATUS_WATCHDOG_RESET:
		Serial.print(F("Watchdog reset "));
		{
			static unsigned int time = 0;
			Serial.print((millis()-time)/1000,DEC);
			time=millis();
		}
		Serial.print("s ");
		break;
	case XBEE_MODEM_STATUS_JOINED:
		Serial.print(F("Joined"));
		break;
	case XBEE_MODEM_STATUS_NO_LONGER_JOINED:
		Serial.print(F("No longer joined"));
		break;
	case XBEE_MODEM_STATUS_IP_CONFIG_ERROR:
		Serial.print(F("IP configuration error"));
		break;
	case XBEE_MODEM_STATUS_S_OR_J_WITHOUT_CON:
		Serial.print(F("Send or join without connecting first"));
		break;
	case XBEE_MODEM_STATUS_AP_NOT_FOUND:
		Serial.print(F("AP not found"));
		break;
	case XBEE_MODEM_STATUS_PSK_NOT_CONFIGURED:
		Serial.print(F("Key not configured"));
		break;
	case XBEE_MODEM_STATUS_SSID_NOT_FOUND:
		Serial.print(F("SSID not found"));
		break;
	case XBEE_MODEM_STATUS_FAILED_WITH_SECURITY:
		Serial.print(F("Failed to join with security enabled"));
		break;
	case XBEE_MODEM_STATUS_INVALID_CHANNEL:
		Serial.print(F("Invalid channel"));
		break;
	case XBEE_MODEM_STATUS_FAILED_TO_JOIN:
		Serial.print(F("Failed to join AP"));
		break;
	default:
		Serial.print(F("Unknown Status Code"));
		break;
	}
	Serial.println(F(")"));
}

int wifi_init(uint8_t prot) {
	nointerrupt;

	tcp_or_udp_flag = prot;
	uint8_t result = 0;
	xbee.register_ip_data_callback(inbound_ip);
	xbee.register_status_callback(inbound_status);
for(;;)
	{
	result = xbee.init(XBEE_SELECT, XBEE_ATN, XBEE_RESET, XBEE_DOUT);
	 if(result!=0) break;
	 if (tcp_or_udp_flag == TCPIP) {
	 			result &= atbyte(XBEE_AT_NET_IPPROTO, XBEE_NET_IPPROTO_TCP);
	 		}
	 		if (tcp_or_udp_flag == UDP) {
	 			result &= atbyte(XBEE_AT_NET_IPPROTO, XBEE_NET_IPPROTO_UDP);
	 		}
	 if(result!=0) break;
#ifndef AT_PRECONFIGURED
	 result &= atbyte("P3", 1);
	if(result!=0) break;
	result &= atbyte("D7", 1);
	 if(result!=0) break;
	result &= atbyte("P4", 1);
	 if(result!=0) break;
	result &= write_AT_config();
	 if(result!=0) break;
	break;
#endif
	}
	xbee.process();
	interrupt;
	return result;
}

//verbindet mit dem AP essid
int wifi_connecttoAp(const char* essid) {
	/* CE 2
	 * AH 2
	 * ID essid
	 */
	uint8_t result = 0;
	if (modemstatus != XBEE_MODEM_STATUS_JOINED) {
#ifndef AT_PRECONFIGURED
		nointerrupt;
		for(;;){
		result &= atbyte(XBEE_AT_INFRASTRUCTURE_MODE,
				XBEE_NET_TYPE_IBSS_INFRASTRUCTURE);
		 if(result!=0) break;
		result &= atbyte(XBEE_AT_NET_TYPE, XBEE_NET_TYPE_IBSS_INFRASTRUCTURE);
		 if(result!=0) break;
		result &= atstring(XBEE_AT_NET_SSID, essid);
		 if(result!=0) break;
		result &= atbyte(XBEE_AT_NET_ADDRMODE, XBEE_NET_ADDRMODE_DHCP);
		 if(result!=0) break;
		result &= atbyte(XBEE_AT_NET_IPPROTO, XBEE_NET_IPPROTO_TCP);
		 if(result!=0) break;
		result &= atbyte(XBEE_AT_NET_TCP_TIMEOUT, 0x64);
		 if(result!=0) break;
		result &= atbyte(XBEE_AT_SERIAL_API_ENABLE,XBEE_SERIAL_API_ENABLE_NOESC);
		 if(result!=0) break;
		result &= write_AT_config();
		 break;

		}
		xbee.process();
		interrupt;
#endif
	}
	else
		return -10;
	return result;
}

//erstellt eine tcp oder udp verbindung zum server mit der ip und dem port her
int wifi_connecttoServer(unsigned char* ip, uint16_t port) {
	nointerrupt;

	/* C0 0
	 * IP 1
	 * DE port
	 * DL ip
	 */
	remote_port = port;
	port = ((port & 0xFF) << 8) + ((port & 0xFF00) >> 8);

	remote_ip = (char*) ip;

	uint8_t result = 0;
#ifndef AT_PRECONFIGURED
	for(;;){
	result &= atshort(XBEE_AT_ADDR_SERIAL_COM_SERVICE_PORT, 0);
	 if(result!=0) break;
	result &= atshort(XBEE_AT_ADDR_DEST_PORT, port);
	 if(result!=0) break;
	result &= atstring(XBEE_AT_ADDR_DEST_ADDR, (const char*) ip);
	 if(result!=0) break;
	result &= write_AT_config();
	break;
	}
#endif
	xbee.process();
	interrupt;
	return result;
}

//sendet an die letzte erstellte verbindung
int wifi_senddata(uint8_t* data, int datalength) {

	nointerrupt;
	int result = 0;

	s_txoptions txopts;
	txopts.dest_port = remote_port;
	txopts.source_port = 0;
	txopts.leave_open=true;

	if (tcp_or_udp_flag == TCPIP) {
		txopts.protocol = XBEE_NET_IPPROTO_TCP;
	}

	if (tcp_or_udp_flag == UDP) {
		txopts.protocol = XBEE_NET_IPPROTO_UDP;
		result &= atbyte(XBEE_AT_NET_IPPROTO, XBEE_NET_IPPROTO_UDP);
	}

#ifdef WIFI_DELAY
	static unsigned int lastsend = 0;
	while(millis()-lastsend<(unsigned int)datalength/2);

	Serial.println(F("--------------SENDING-------------"));
	Serial.print(F("length: "));
	Serial.println(datalength,DEC);
	Serial.print(F("time: "));
	Serial.println(millis()-lastsend);
	Serial.println(F("----------------------------------"));
#endif

//if(tcp_or_udp_flag==TCPIP)
	result = xbee.transmit((uint8_t *) remote_ip, &txopts, data, datalength,true); //solid
	if(result == 0xEE){
		result = xbee.transmit((uint8_t *) remote_ip, &txopts, data, datalength,true);
	}
/*else
	result = xbee.transmit((uint8_t *)remote_ip, &txopts, data, datalength,false); //testing
*/
#ifdef WIFI_DELAY
	lastsend = millis();
#endif

	xbee.process();
	interrupt;
	return result;
}


unsigned char betterstatus() {
	nointerrupt;

	unsigned char ret = 0;
	int len;
	xbee.at_query("AI", &ret, &len, 1);

	if (ret == 0)
		modemstatus = XBEE_MODEM_STATUS_JOINED;

	xbee.process();
	interrupt;

	Serial.print(F("[wifi] status: "));
	Serial.println(ret,HEX);

	if (len > 1)
		return 0xEE;
	return ret;
}

//sendet ein reset befehl an das xbee
int wifi_reset() {

	Serial.println(F("[wifi] doing reset"));
	nointerrupt;

	pinMode(XBEE_RESET, OUTPUT);
	digitalWrite(XBEE_RESET, LOW);
	delay(1000);
	digitalWrite(XBEE_RESET, HIGH);
	//pinMode(XBEE_RESET, INPUT_PULLUP);

	xbee.process();
	interrupt;
	delay(2000);
	return 0;
}

int wifi_isready() {
	if (modemstatus != XBEE_MODEM_STATUS_JOINED)
		return modemstatus + 1;
	else
		return 0;
}

double speedtest(int packagesize, int numberofpackages) {
	int starttime = millis();

	for (int x = 0; x < numberofpackages; x++) {
		Serial.print(".");
		uint8_t tmp[packagesize];
		//void* tmp = malloc(packagesize);
		/*if(!tmp)
			ERROR_MESSAGE("[wifi] allocation error\r\n");
*/
		memset(tmp, 'A' , packagesize);

		int i = wifi_senddata((uint8_t*) tmp, packagesize);

		free(tmp);

		if (i != 0) {
			return -i;
		}
	}

	return (double) (packagesize * numberofpackages * 8)
			/ (double) (millis() - starttime);
}

void print_xbee_config() {
	uint8_t buffer[20];
	int len = 0;

	const char* AT_befehle[] = { "DL", "MY", "MK", "GW", "SH", "SL", "NI", "DE",
			"C0", "DD", "NP", "ID", "AH", "IP", "MA", "TM", "EE", "PK", "PL",
			"CH", "AP", "BD", "NB", "SB", "RO", "FT", "P0", "P1", "P2", "P3",
			"P4", "D0", "D1", "D2", "D3", "D4", "D5", "D7", "D6", "D8", "D9",
			"LT", "PR", "PD", "AV", "M0", "M1", "VR", "HV", "AI", "TP", "CK",
			"%V", "CT", "GT", "CC", "SM", "SP", "SO", "WH", "ST" };

	nointerrupt;

	for (int i = 0; i < 61; i++) {
		len = 0;
		xbee.at_query(AT_befehle[i], buffer, &len, 20);
		Serial.print(AT_befehle[i]);
		Serial.print(": ");
		if(len==4)
		{
			for (int c = 0; c < len; c++){
					Serial.print(buffer[c], DEC);
					Serial.print(".");
			}
		}
		else
		{
			for (int c = 0; c < len; c++)
				Serial.print(buffer[c], HEX);
		}
		Serial.println("");
	}

	xbee.process();
	interrupt;
}





char* dns_lookup(const char* domain) //DOES NOT WORK TODO!!!!!!!!!
		{
	nointerrupt;


	char ip_buffer[20];

	int size = 0;
	xbee.at_query(XBEE_AT_DIAG_FIRMWARE_VERSION, (uint8_t*) ip_buffer, &size,
			20);
	Serial.print(size);
	Serial.print(",");
	Serial.print(ip_buffer[0], HEX);
	Serial.print(",");
	Serial.println(ip_buffer[1], HEX);

	Serial.println(
			xbee.at_query_str(XBEE_AT_DNS_LOOKUP, (uint8_t*) domain,
					strlen(domain), (uint8_t*) ip_buffer, &size, 20));
	Serial.print(size);
	Serial.print(" ip: ");
	Serial.print(ip_buffer[0], DEC);
	Serial.print(".");
	Serial.print(ip_buffer[1], DEC);
	Serial.print(".");
	Serial.print(ip_buffer[2], DEC);
	Serial.print(".");
	Serial.println(ip_buffer[3], DEC);
	xbee.process();
	interrupt;
	return 0;
}
