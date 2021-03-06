/*
 * wifi.h
 *
 *  Created on: 02.06.2015
 *      Author: alexander
 */

#ifndef WIFI_H_
#define WIFI_H_

#include "XBee.h"
#include "Arduino.h"


#define TCPIP 1
#define UDP 2

//pinconfiguration for non spi pins
#define XBEE_SELECT 10
#define XBEE_ATN 	9
#define XBEE_RESET  15
#define XBEE_DOUT   0


//init periphery to communicate with xbee.
//returns 0 for success
int wifi_init(uint8_t prot);

//setup connection to accesspoint with the name essid. No encryption supported yet
//returns 0 for success
int wifi_connecttoAp(const char* essid);

//setup TCP connection to Server (only one connection possible)
//returns 0 for success
int wifi_connecttoServer(unsigned char* ip,uint16_t port);

//send data with datalength length to server (if not connected return >0)
int wifi_senddata(uint8_t* data, int datalength);

//toggle resetline on xbee to trigger reset
int wifi_reset();

//checks xbee status
//returns 0 if connection is established
int wifi_isready();

char* dns_lookup(const char* domain); // ----------------TODO

//try to send numberofpackages random packages of size packagesize as fast as possible
double speedtest(int packagesize,int numberofpackages);

//outputs all AT configuration of xbee
void print_xbee_config();

//request status from xbee
unsigned char betterstatus();

//makes xbee factory reset
void factory_reset();


//should not be used!!!
void attn_interrupt();


int get_last_message_time();


#endif /* WIFI_H_ */
