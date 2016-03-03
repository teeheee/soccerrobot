/*
 * led.cpp
 *
 *  Created on: 10.06.2015
 *      Author: alexander
 */



#include "led.h"

IntervalTimer Timer;

volatile uint8_t led_green;
volatile uint8_t led_red;

void ledToggle(uint8_t color);

#define STARTINT Timer.begin(led_interrupt, 250000);
#define STOPINT  Timer.end()


void led_interrupt()
{
	if(led_green>0)
		ledToggle(LED_GREEN);
	if(led_red>0)
		ledToggle(LED_RED);
}




void ledinit()
{
	led_red = 0;
	led_green = 0;
	pinMode(LED_RED,OUTPUT);
	pinMode(LED_GREEN,OUTPUT);
	Timer.priority(PRIORITY_LED);
	STARTINT;
}

void ledBlinkOn(uint8_t color)
{
	STOPINT;
	ledOff(color);
	if(color==LED_RED)
		led_red = 1;
	if(color==LED_GREEN)
		led_green = 1;
	STARTINT;
}

void ledBlinkOff(uint8_t color)
{
	STOPINT;
	if(color==LED_RED)
		led_red = 0;
	if(color==LED_GREEN)
		led_green = 0;
	ledOff(color);
	STARTINT;
}

void ledOn(	uint8_t color)
{
	STOPINT;
	if(color==LED_RED)
		led_red = 0;
	if(color==LED_GREEN)
		led_green = 0;
	digitalWrite(color,HIGH);
	STARTINT;
}

void ledToggle(uint8_t color)
{
	STOPINT;
	digitalWrite(color,!digitalRead(color));
	STARTINT;
}

void ledOff(uint8_t color)
{
	STOPINT;
	if(color==LED_RED)
		led_red = 0;
	if(color==LED_GREEN)
		led_green = 0;
	digitalWrite(color,LOW);
	STARTINT;
}
