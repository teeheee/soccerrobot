/*
 * wifistream.cpp
 *
 *  Created on: 04.08.2015
 *      Author: alexander
 */

#include "wifistream.h"
#include "config.h"
#include <stdlib.h>

//#define alternate
//#define usb
//#define DEBUGPIN


#ifdef DEBUGPIN
#define STARTWORKING digitalWrite(DEBUG_PIN,HIGH)
#define STOPWORKING digitalWrite(DEBUG_PIN,LOW)
#else
#define STARTWORKING
#define STOPWORKING
#endif


#ifndef usb
	//#define WIFISTREAM_ENABLE_DEBUG
#endif
// Debug functions
#ifdef WIFISTREAM_ENABLE_DEBUG
// Debug is enabled, XBEE_DEBUG is a simple macro that just inserts the code within it's parameter
#define WIFISTREAM_DEBUG(x) (x)
#else
// Debug is not enabled, the XBEE_DEBUG becomes a NOP macro that essentially discards it's parameter
#define WIFISTREAM_DEBUG(x)
#endif


volatile int buffer_usage;
volatile char stream_buffer[BUFFER_SIZE];

   output_stream::output_stream()
   {
	   buf_ofs = 0;
	   init_buffer();

#ifdef DEBUGPIN
  pinMode(DEBUG_PIN,OUTPUT);
#endif
   }

   output_stream::~output_stream(){}

   void output_stream::init_buffer()
   {
	   WIFISTREAM_DEBUG(Serial.println(F("buffer auf null")));
#ifdef usb
	   buffer_usage=0;
#else
	   buffer_usage=sizeof(struct status);
	   struct status s;
	   generatestatusmessage(s);
	   s.bildlaenge = BUFFER_SIZE-sizeof(struct status);
	   memcpy((void*)stream_buffer,(const void*)&s,sizeof(struct status));
#endif
	   WIFISTREAM_DEBUG(Serial.print(F("added header -> new buffer_usage: ")));
	   WIFISTREAM_DEBUG(Serial.println(buffer_usage,DEC));
   }

   bool output_stream::put_buf(const void* pBuf, int len)
   {
	   STARTWORKING
	   WIFISTREAM_DEBUG(Serial.print(F("input: ")));
	   WIFISTREAM_DEBUG(Serial.println(len,DEC));
	 if(len+buffer_usage > BUFFER_SIZE)
     {
		 WIFISTREAM_DEBUG(Serial.println(F("buffer voll!!")));

		 ledOn(LED_RED);
    	 int restlength = BUFFER_SIZE-buffer_usage;

    	 WIFISTREAM_DEBUG(Serial.print(F("letzter paket teil: ")));
    	 WIFISTREAM_DEBUG(Serial.println(restlength,DEC));

    	 memcpy((void*)(stream_buffer+buffer_usage),(const void*)(pBuf),restlength);
    	 buf_ofs += restlength;
#ifndef alternate
#ifdef usb
    	 for(int x = 0; x < BUFFER_SIZE;x++)
    		 Serial.write(stream_buffer[x]);
#else
    	 if(wifi_isready())
    	 {
      		 ledOff(LED_RED);
      		ERROR_MESSAGE("[wifistream] wifi is not ready breaking!!!\r\n\r\n");
    		 return false;
    	 }
		int ret = wifi_senddata((unsigned char*)stream_buffer,BUFFER_SIZE);
    	 while(ret==3)
    	 {
    		 ERROR_MESSAGE("[wifistream] watch out!!! xbee buffer error 3!!\r\n\r\n");
    		 delay(100);
    		 ret = wifi_senddata((unsigned char*)stream_buffer,BUFFER_SIZE);
    	 }
    	 WIFISTREAM_DEBUG(Serial.println(ret));
    	 if(ret!=0)
    	 {
				ledOff(LED_RED);
				ERROR_MESSAGE("[wifistream] some other return code: ");
				ERROR_MESSAGE(ret);
				ERROR_MESSAGE("\r\n\r\n");
    		 	return false;
    	 }
#endif
#endif
    	 init_buffer();
    	 put_buf((const void*)((int)pBuf+restlength),len-restlength);
    	 STARTWORKING;
  		 ledOff(LED_RED);
     }
     else
     {

    	 WIFISTREAM_DEBUG(Serial.print(F("moving memory ")));
    	 memcpy((void*)((int)stream_buffer+buffer_usage),(const void*)pBuf,len);
    	 buffer_usage+=len;
    	 buf_ofs += len;
     }
	 WIFISTREAM_DEBUG(Serial.print(F("new buffer_usage: ")));
	 WIFISTREAM_DEBUG(Serial.println(buffer_usage,DEC));
	 STOPWORKING;
     return true;
   }


   uint output_stream::get_size() const
   {
      return buf_ofs;
   }

   void output_stream::reset_size()
      {
         buf_ofs=0;
      }
