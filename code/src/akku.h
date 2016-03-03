/*
 * akku.h
 *
 *  Created on: 10.06.2015
 *      Author: alexander
 */

#ifndef AKKU_H_
#define AKKU_H_

#include "Arduino.h"
#include "config.h"

#define PIN_CELL1 27
#define PIN_CELL2 28

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



#endif /* AKKU_H_ */
