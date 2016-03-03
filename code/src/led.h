/*
 * led.h
 *
 *  Created on: 10.06.2015
 *      Author: alexander
 */

#ifndef LED_H_
#define LED_H_

#include "Arduino.h"
#include "config.h"

#define LED_RED 33
#define LED_GREEN 31

// initialise leds
void ledinit();


//activate blinking
void ledBlinkOn(uint8_t color);
void ledBlinkOff(uint8_t color);

// turn on and off
void ledOn(uint8_t color);
void ledOff(uint8_t color);


#endif /* LED_H_ */
