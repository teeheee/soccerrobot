#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "serialutils.h"
#include "wifi.h"
#include "sensors.h"
#include "akku.h"

//interrupt priority
// 0 = highest priority
// Cortex-M4: 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224,240

/* on Teensy all the interrupts default to priority 128.
 * USB defaults to 112, the hardware serial ports default to 64,
 * and systick defaults to 0.*/

#define PRIORITY_OV7670 42
#define PRIORITY_XBEE 200
#define PRIORITY_LED 224



//uni-ulm.de/~knb80/ip.html

// some relicts from previous versions....
#define VIDEO
#define MOTORS
#define DOF9_SENSOR
#define SEND_WIFI

//#define ROBOTERSCHWARZ
#define ROBOTERGELB


#define DEBUG_PIN 29


// will be saved in eeprom
struct config{
	uint8_t ip[4];
	uint8_t tcp_or_ip;
	uint16_t port;
	uint8_t motordirleft;
	uint8_t motordirright;
	uint8_t motorminspeedleft;
	uint8_t motorminspeedright;
	uint16_t irtorwert;
	uint8_t validflag;
};


// package reseived from server
struct befehl{
uint8_t startbyte;
uint8_t kamera;
uint8_t kicker;
uint8_t motorlinks;
uint8_t motorrechts;
uint8_t checksum;
} __attribute__ ((packed));

// package send to server
struct status{
uint8_t roboterid;
uint8_t akkustand;
uint8_t irsensor;
uint16_t kompasssensor;
uint16_t xposition;
uint16_t yposition;
uint8_t fehlercode;
uint16_t bildlaenge;
} __attribute__ ((packed));



//handle camera state (activate -> 1, deactivate -> 0) (should only be called in main!!!)
void activate_camera(uint8_t state);

//returns current camera state
uint8_t getcamerastate();

//generates status message from sensor values
void generatestatusmessage(struct status &s);

//saves config struct in eeprom
void saveeeprom(struct config &c);

//loads config struct from eeprom (standard value is defined in this function)
void loadeeprom(struct config* c);


#endif


