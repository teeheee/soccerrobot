#include "aktuators.h"


uint8_t	motor_left_direction;
uint8_t	motor_right_direction;


uint8_t	motor_left_minspeed;
uint8_t	motor_right_minspeed;


void initaktuators()
{
	motor_left_direction = 0;
	motor_right_direction = 0;
	motor_left_minspeed=0;
	motor_right_minspeed=0;

  analogWriteResolution(7);
  analogWriteFrequency(PIN_MOTOR11,1000);
  analogWriteFrequency(PIN_MOTOR10,1000);
  analogWriteFrequency(PIN_MOTOR21,1000);
  analogWriteFrequency(PIN_MOTOR20,1000);

  pinMode(PIN_MOTOR11,OUTPUT);
  pinMode(PIN_MOTOR10,OUTPUT);
  pinMode(PIN_MOTOR21,OUTPUT);
  pinMode(PIN_MOTOR20,OUTPUT);
  pinMode(PIN_KICK,OUTPUT);

}

void kick() //TODO remove delay
{
  digitalWrite(PIN_KICK,1);
  delay(50);
  digitalWrite(PIN_KICK,0);
}

#define SPEED_PER_RES 125.0f/100.0f

void motors(int8_t nspeed1,int8_t nspeed2)
{
	nspeed1 = motor_right_direction?-nspeed1:nspeed1;
	nspeed2 = motor_left_direction?-nspeed2:nspeed2;


	if(abs(nspeed1)<motor_right_minspeed && nspeed1!=0)
		nspeed1 = nspeed1>0?motor_right_minspeed:-motor_right_minspeed;

	if(abs(nspeed2)<motor_left_minspeed && nspeed2!=0)
		nspeed2 = nspeed2>0?motor_left_minspeed:-motor_left_minspeed;


	if(nspeed1>0)
	{
		  analogWrite(PIN_MOTOR11,nspeed1*SPEED_PER_RES);
		  analogWrite(PIN_MOTOR10,0);
	}
	else
	{
		  analogWrite(PIN_MOTOR10,-nspeed1*SPEED_PER_RES);
		  analogWrite(PIN_MOTOR11,0);
	}

	if(nspeed2>0)
	{
		  analogWrite(PIN_MOTOR21,nspeed2*SPEED_PER_RES);
		  analogWrite(PIN_MOTOR20,0);
	}
	else
	{
		  analogWrite(PIN_MOTOR20,-nspeed2*SPEED_PER_RES);
		  analogWrite(PIN_MOTOR21,0);
	}
}



void setdirection(uint8_t motorleft, uint8_t motorright)
{
#ifdef ROBOTERSCHWARZ
	motor_left_direction = motorleft;
	motor_right_direction = motorright;
#endif
#ifdef ROBOTERGELB
	motor_left_direction = ~motorleft;
	motor_right_direction = motorright;
#endif
}

int getminimalspeed(uint8_t motor_number)
{
	Serial.println("press when robot is moving");
	int s = 0;
	motor_left_minspeed = 0;
	motor_right_minspeed = 0;

	while(Serial.available()==0)
	{
		s++;
		Serial.println(s);
		if(motor_number)
			motors(s,0);
		else
			motors(0,s);
		delay(500);
	}
	motors(0,0);
	return s;
}

void setminimalspeed(uint8_t motorleft, uint8_t motorright)
{
  motor_left_minspeed = motorleft;
  motor_right_minspeed = motorright;
}



