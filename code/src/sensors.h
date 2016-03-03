/*
 * sensors.h
 *
 *  Created on: 11.06.2015
 *      Author: alexander
 */

#ifndef SENSORS_H_
#define SENSORS_H_

#include "Arduino.h"
#include "config.h"
#include "led.h"

//ir sensor pin
#define PIN_IR 24

//batteri cell pins
#define PIN_CELL1 27
#define PIN_CELL2 28


//init all sensors connected to teensy
void init_sensors();

//outputs pulse length in microseconds
uint32_t read_ir();


//initialise the ADC-Port for lipo voltage messurment
void init_akku();

//get voltage of lipo cell 1
double akku_zelle1();

//get voltage of lipo cell 2
double akku_zelle2();

//returns lipo energy level
double akku_percent();

//returns voltage difference of lipo cells. Should never exceed 0,1V
double akku_zellendif();




#endif /* SENSORS_H_ */

