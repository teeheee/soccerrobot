#ifndef _OV7670_
#define _OV7670_

#include "Arduino.h"
#include "sccb.h"


// image format QCIF,QVGA,CIF,VGA (VGA is still not working)
#define CIF

#ifdef QCIF

#define LINEBUFFERSIZE 20
#define WIDTH 174*2
#define HEIGHT 144
#endif
#ifdef QVGA

#define LINEBUFFERSIZE 16
#define WIDTH 320*2
#define HEIGHT 240
#endif
#ifdef CIF //352x288

#define LINEBUFFERSIZE 8
#define WIDTH (352)*2
#define HEIGHT 288
#endif
#ifdef VGA //352x288
#define LINEBUFFERSIZE 4
#define WIDTH 640*2
#define HEIGHT 480
#endif

//the size of one raw image
#define BUFFERSIZE LINEBUFFERSIZE*WIDTH
//the size of one line of a raw image
#define DMA_DATALENGTH WIDTH


#define DO_PIN 2
#define D1_PIN 14
#define D2_PIN 7
#define D3_PIN 8
#define D4_PIN 6
#define D5_PIN 20
#define D6_PIN 21
#define D7_PIN 5
#define HREF_PIN 4 //(PTA13)
#define VSYNC_PIN 17 //(PTC2)
#define XCLK_PIN 3
#define PCLK_PIN 26


//initalise the camera
void init_ov7670();

//returns a pointer to the line index
uint8_t* getline(int index);

// free line to save ram and to get a new line
void freeline(int index);

//stop and start driver if something went wrong
void startimageandfreeram();
void stopimageandfreeram();

//some debug functions...
void dummyprocessor();
void printdebug();
double getfps();

//change configuration of ov7670 registers over sccb
int changeregister(uint8_t adress,uint8_t data);

#endif


