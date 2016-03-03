#ifndef _SCCB_
#define _SCCB_

#include "Arduino.h"
#include "config.h"

#define ADDRESSE 0x21
#define DATA_PIN 18
#define CLK_PIN  19

#define DELAY delay(1)

class sccb{
public:
  sccb()
  {
    pinMode(CLK_PIN, OUTPUT); 
    pinMode(DATA_PIN, OUTPUT);  
  }
  void sccb_start()
  {
    digitalWrite(DATA_PIN,1);
    DELAY;
    digitalWrite(CLK_PIN,1);
    DELAY;
    digitalWrite(DATA_PIN,0);
    DELAY;
    digitalWrite(CLK_PIN,0);
    DELAY;
  }
  void sccb_stop()
  {
    digitalWrite(DATA_PIN,0);
    DELAY;
    digitalWrite(CLK_PIN,1);
    DELAY;
    digitalWrite(DATA_PIN,1);
    DELAY;
  }
  void sccb_nack()
  {
    digitalWrite(DATA_PIN,1);
    DELAY;
    digitalWrite(CLK_PIN,1);
    DELAY;
    digitalWrite(CLK_PIN,0);
    DELAY;
    digitalWrite(DATA_PIN,0);
    DELAY;

  }
  uint8_t sccb_write(uint8_t data)
  {
    uint8_t outBits,sucess;
    /* 8 bits */
    for(outBits = 0; outBits < 8; outBits++)
    {
      if(data & 0x80)
        digitalWrite(DATA_PIN,1);
      else
        digitalWrite(DATA_PIN,0);
      DELAY;
      data  <<= 1;

      digitalWrite(CLK_PIN,1);
      DELAY;
      digitalWrite(CLK_PIN,0);
      DELAY;
    }
    pinMode(DATA_PIN,INPUT);
    DELAY;
    digitalWrite(CLK_PIN,1);
    DELAY;
    if(digitalRead(DATA_PIN))
      sucess = 0;
    else
      sucess = 1;

    digitalWrite(CLK_PIN,0);
    DELAY;
    pinMode(DATA_PIN,OUTPUT);
    DELAY;
    return sucess;
  }

  uint8_t sccb_read()
  {
    unsigned char inData, inBits;
    pinMode(DATA_PIN,INPUT);
    DELAY;
    inData = 0x00;
    /* 8 bits */
    for(inBits = 0; inBits < 8; inBits++)
    {
      inData <<= 1;
      digitalWrite(CLK_PIN,1);
      DELAY;
      inData |= digitalRead(DATA_PIN);
      digitalWrite(CLK_PIN,0);
      DELAY;
    }

    pinMode(DATA_PIN,OUTPUT);
    DELAY;
    return inData;
  }

  uint8_t sccb_readbyte(uint8_t addresse)
  {
    uint8_t inData;
    sccb_start();

    if (sccb_write(2 * ADDRESSE)) {
      if (sccb_write(addresse)) {
        sccb_stop();
        sccb_start();

        if (sccb_write(2 * ADDRESSE + 1)) {
          inData = sccb_read();

          sccb_nack();

          sccb_stop();
          return inData;
        } 
        else {
          sccb_stop();
          return 1;
        }
      } 
      else {
        sccb_stop();
        return 2;
      }
    } 
    else {
      sccb_stop();
      return 3;
    }
  }
  void sccb_writebyte(uint8_t addresse,uint8_t wert)
  {
    sccb_start();
    if (sccb_write(2 * ADDRESSE)) {
      if (sccb_write(addresse)) {
        if (sccb_write(wert)) {
          sccb_stop();
          return;
        } 
        else {
          sccb_stop();
          return;
        }
      } 
      else {
        sccb_stop();
        return;
      }
    } 
    else {
      sccb_stop();
      return;
    }
  }
};

#endif







