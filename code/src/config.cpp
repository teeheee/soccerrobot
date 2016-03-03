/*
 * config.cpp
 *
 *  Created on: 09.07.2015
 *      Author: alexander
 */
#include "config.h"

const unsigned char ip_patrick[] = { 134, 60, 143, 43 }; //patrick
const unsigned char ip_alex[] = { 134,60,158,215 }; //ich

struct config* conf;

volatile uint8_t camerastate;

void saveeeprom(struct config &c)
{
	c.validflag=0x42;
	for (uint32_t x = 0; x < sizeof(config); x++)
		eeprom_write_byte((unsigned char*) x, *((char*)(&c)+x));
	conf = &c;
}



void loadeeprom(struct config* c)
{
	char* ptr = (char*)c;
	for (uint32_t x = 0; x < sizeof(config); x++)
			ptr[x] = eeprom_read_byte((const unsigned char*) x);
	if (c->validflag!=0x42) {
			for (int x = 0; x < 4; x++)
				c->ip[x] = ip_alex[x];
			c->port=44044;
			c->tcp_or_ip=UDP;
			c->motorminspeedleft=0;
			c->motorminspeedright=0;
			c->irtorwert=900;
	}
	conf = c;
}


void generatestatusmessage(struct status &s)
{
	s.akkustand = akku_percent();
	uint32_t irwert = read_ir();
	if(abs(conf->irtorwert-irwert)<150)
		s.irsensor = 1;
	else if(abs(conf->irtorwert*2-irwert)<150)
		s.irsensor = 2;
	else if(abs(conf->irtorwert*3-irwert)<150)
		s.irsensor = 3;
	else if(abs(conf->irtorwert*4-irwert)<150)
		s.irsensor = 4;
	else if(abs(conf->irtorwert*5-irwert)<150)
		s.irsensor = 5;
	else if(abs(conf->irtorwert*6-irwert)<150)
		s.irsensor = 6;
	else
		s.irsensor = 0;
	s.kompasssensor = 0;
#ifdef ROBOTERSCHWARZ
	s.roboterid=0x01;
#endif
#ifdef ROBOTERGELB
	s.roboterid=0x02;
#endif
	static uint16_t statusnumber = 0;
	statusnumber++;
	s.xposition=statusnumber;
	s.yposition=0;
	s.fehlercode=0;
	s.bildlaenge=0;
}


void activate_camera(uint8_t state)
{
	static uint8_t tmp = 0;
	if(tmp!=state)
	{
		camerastate = state;
		tmp=state;
	}
}

uint8_t getcamerastate()
{
	uint8_t tmp = camerastate;
	return tmp;
}
