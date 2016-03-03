/*
 * Serialutils.h
 *
 *  Created on: 02.07.2015
 *      Author: alexander
 */

#ifndef SERIALUTILS_H_
#define SERIALUTILS_H_

#include "Arduino.h"


#define ERROR_MESSAGE(x) Serial.write(27);Serial.print(F("[31m"));Serial.print(F(x));Serial.write(27);Serial.print(F("[0m"))


void clearline();

void clearscreen();

void waitforbutton();

void printip(unsigned char* ip);

void readip(unsigned char* ip);

int getint();

void flushinput();

#endif /* SERIALUTILS_H_ */
