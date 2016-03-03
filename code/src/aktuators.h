#ifndef _aktuators_
#define _aktuators_

#include "Arduino.h"
#include "config.h"


//pinconfiguration for motorcontroll
//if motor direction can't be controlled try to change pin_motor11 and pin_motor21.
#define PIN_KICK 16
#define PIN_MOTOR11 23
#define PIN_MOTOR10 22
#define PIN_MOTOR21 32
#define PIN_MOTOR20 25

//init timer for pwm mode
void initaktuators();

//triggers kick (blocking function!!!)
void kick();

//motorleft/right=1 inverts motor direction
//motorleft/right=0 leaves motor direction like it is
void setdirection(uint8_t motorleft, uint8_t motorright);

//change motorspeed (-125 to +125)
void motors(int8_t nspeed1,int8_t nspeed2);


//sets minimal speed for motor. for 0<speed<minimalspeed speed=minimalspeed
void setminimalspeed(uint8_t motorleft, uint8_t motorright);

//returns minimalspeed
int getminimalspeed(uint8_t motor_number);

#endif





