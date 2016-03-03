#include "sensors.h"

//#define alternate

volatile uint32_t pulsewidth;


void pulseinterrupt()
{
	static int i = 0;
	static uint32_t time = 0;
	if(i==0)
	{
		attachInterrupt(PIN_IR,pulseinterrupt,RISING);
		time=micros();
	}
	else
	{
		attachInterrupt(PIN_IR,pulseinterrupt,FALLING);
		pulsewidth=micros()-time;
	}
	i^=1;
}

void init_sensors()
{
	pinMode(PIN_IR,INPUT);
#ifndef alternate
	attachInterrupt(PIN_IR,pulseinterrupt,FALLING);
#endif
}


uint32_t read_ir()
{
#ifdef alternate
	return 0;
#endif
	detachInterrupt(PIN_IR);
	uint32_t tmp = pulsewidth;
	pulsewidth=0;
	attachInterrupt(PIN_IR,pulseinterrupt,FALLING);
	return tmp;
}



void init_akku()
{
	pinMode(PIN_CELL1,INPUT);
	pinMode(PIN_CELL2,INPUT);
	for(int x = 0; x < 10; x++){
		akku_percent();
		akku_zellendif();
	}
}

double akku_zelle1()
{
#ifdef ROBOTERSCHWARZ
	return (analogRead(PIN_CELL1)/320.0f)*(1.8);
#endif
#ifdef ROBOTERGELB
	return (analogRead(PIN_CELL1)/310.0f)*(2);
#endif
}

double akku_zelle2()
{
#ifdef ROBOTERSCHWARZ
	return (analogRead(PIN_CELL2)/320.0f)*(1.5);
#endif
#ifdef ROBOTERGELB
	return (analogRead(PIN_CELL2)/310.0f)*(1.5);
#endif
}


#define MAX_VOLTAGE 4.2f
#define MIN_VOLTAGE 3.6f
//returns -1 for error


double akku_percent()
{
	static int ret[10];
	static int count = 0;
	double z1 = akku_zelle1();
	double z2 = akku_zelle2();
	ret[count] = (((z1+z2)-MIN_VOLTAGE*2)/((MAX_VOLTAGE-MIN_VOLTAGE)*2))*100;
	count = (count+1)%10;
	double mid = 0;
	for(int x = 0; x < 10; x++)
		mid += ret[x];
	mid/=10;
	if(mid > 100)
		mid = 100;
	if(mid < 0)
		mid = 0;
	return mid;
}

double akku_zellendif()
{

	static int ret[10];
	static int count = 0;
	double z1 = akku_zelle1();
	double z2 = akku_zelle2();
	ret[count]=z1-z2;
	if(ret[count] < 0)
		ret[count]=-ret[count];
	count = (count+1)%10;
	double mid = 0;
	for(int x = 0; x < 10; x++)
		mid += ret[x];
	mid/=10;
	return mid;
}
