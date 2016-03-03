
#include "serialutils.h"


void clearline()
{

	  Serial.write(27);       // ESC command
	  Serial.print(F("[1A"));    // move up
	  Serial.write(27);       // ESC command
	  Serial.print(F("[2K"));    // clear line
	  Serial.write(27);
	  Serial.print(F("[500D")); // move to first pos
}

void clearscreen()
{

	  Serial.write(27);       // ESC command
	  Serial.print(F("[2J"));    // move upEsc[2J
	  Serial.write(27);       // ESC command
	  Serial.print(F("[H"));    // clear line
}



void waitforbutton() {
	while (Serial.available() == 0);
	while (Serial.available() > 0)
		Serial.read();
}

void printip(unsigned char* ip)
{
	Serial.print(ip[0], DEC);
	Serial.print(".");
	Serial.print(ip[1], DEC);
	Serial.print(".");
	Serial.print(ip[2], DEC);
	Serial.print(".");
	Serial.print(ip[3], DEC);
}

void readip(unsigned char* ip)
{
	ip[0] = getint();
	ip[1] = getint();
	ip[2] = getint();
	ip[3] = getint();
}

int getint()
{
	char ziffer = 0;
	int zahl = 0;
	do
	{
		zahl=zahl*10+ziffer;
		 while (Serial.available() == 0);
		 ziffer = Serial.read();
		 Serial.print(ziffer);
		 ziffer-=48;
	}
	while(ziffer>=0 && ziffer<=9);
	return zahl;
}

void flushinput()
{
	while (Serial.available() > 0)
		Serial.read();

}



