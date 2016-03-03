
#include "ov7670.h"
#include "DMAChannel.h"
//#include "DmaSpi.h"
#include "config.h"
#include "led.h"
#include "ov7670_REG.h"

#define normal


//#define DEBUG


#ifdef DEBUGPIN
#define STARTWORKING digitalWrite(DEBUG_PIN,HIGH)
#define STOPWORKING digitalWrite(DEBUG_PIN,LOW)
#else
#define STARTWORKING
#define STOPWORKING
#endif

#define PRESCALER 0b00000001
#define PLL		  0b01000000
#define XCLK 	  8000000UL




#define SIZEOFREG 9 //49

#define OV7670_TIMEOUT 5000

const uint8_t OV7725_Reg[SIZEOFREG][2]=
{
		{0x12,0x80}, //RESET
		//{0x42,0x08 | 0b000},
		//{0x42,0x08 | 0b100},//colorbar
		{0x11, (0x80 & 0b0000000) | PRESCALER},//prescaler
		{0x6B, (0x3A & 0b00111111) | PLL},//PLL
		{0x0C, 0 | 0b00001000}, //enable format scaling
#ifdef QCIF
		{0x12, (0 & 0b11000111) | 0b00001000}, //QCIF4
#endif
#ifdef QVGA
		{0x12, (0 & 0b11000111) | 0b00010000}, //QVGA
#endif
#ifdef CIF
		{0x12,  0b00100000}, //QVGA
#endif
#ifdef VGA
		{0x12, (0 & 0b11000111) | 0b00000000}, //QVGA
#endif
		{0x15, (1<<5)}, //PCLK off on href low
		{0x41,18},
		{0x58,0x1E | 0b10000000},
		{0x76,0x01 | 0b01100000},
	/*	{0x4f, 0xc0}, //saturation +2
		{0x50, 0xc0},
		{0x51, 0x00},
		{0x52, 0x33},
		{0x53, 0x8d},
		{0x54, 0xc0},
		{0x58, 0x9e}*/
/*
		{0x7a, 0x20},//21/ SLOP : Gamma Curve Highest Segment Slope
		{0x7b, 0x10},//22/ GAM1 : Gamme Curve 1st Segment
		{0x7c, 0x1e},//23/ GAM2 : Gamme Curve 2st Segment
		{0x7d, 0x35},//24/ GAM3 : Gamme Curve 3st Segment
		{0x7e, 0x5a},//25/ GAM4 : Gamme Curve 4st Segment
		{0x7f, 0x69},//26/ GAM5 : Gamme Curve 5st Segment
		{0x80, 0x76},//27/ GAM6 : Gamme Curve 6st Segment
		{0x81, 0x80},//28/ GAM7 : Gamme Curve 7st Segment
		{0x82, 0x88},//29/ GAM8 : Gamme Curve 8st Segment
		{0x83, 0x8f},//30/ GAM9 : Gamme Curve 9st Segment
		{0x84, 0x96},//31/ GAM10: Gamme Curve 10st Segment
		{0x85, 0xa3},//32/ GAM11: Gamme Curve 11st Segment
		{0x86, 0xaf},//33/ GAM12: Gamme Curve 12st Segment
		{0x87, 0xc4},//34/ GAM13: Gamme Curve 13st Segment
		{0x88, 0xd7},//35/ GAM14: Gamme Curve 14st Segment
		{0x89, 0xe8},//36/ GAM15: Gamme Curve 15st Segment
		//
		// Automatic Gain Control and AEC Parameters
		//
		{0x13, 0x00},//37/ COM8 : Fast AGC/AEC Algorithm
		{0x00, 0x00},//38/ GAIN
		{0x10, 0x00},//39/ AECH
		{0x0d, 0x00},//40/ COM4 :
		{0x14, 0x18},//41/ COM9 : Automatic Gain Ceiling : 8x
		{0xa5, 0x05},//42/ BD50MAX: 50 Hz Banding Step Limit
		{0xab, 0x07},//43/ BD60MAX: 60 Hz Banding Step Limit
		{0x24, 0x95},//44/ AGC - Stable Operating Region Upper Limit
		{0x25, 0x33},//45/ AGC - Stable Operating Region Lower Limit
		{0x26, 0xe3},//46/ AGC - Fast Mode Operating Region
		{0x9f, 0x78},//47/ HAECC1 : Histogram based AEC Control 1
		{0xa0, 0x68},//48/ HAECC2 : Histogram based AEC Control 2
		{0xa1, 0x03},//49/ Reserved
		{0xa6, 0xd8},//50/ HAECC3 : Histogram based AEC Control 3
		{0xa7, 0xd8},//51/ HAECC4 : Histogram based AEC Control 4
		{0xa8, 0xf0},//52/ HAECC5 : Histogram based AEC Control 5
		{0xa9, 0x90},//53/ HAECC6 : Histogram based AEC Control 6
		{0xaa, 0x94},//54/ HAECC7 : AEC Algorithm Selection
		{0x13, 0xe5},//55/ COM8 : Fast AGC Algorithm, Unlimited Step Size , Banding Filter ON, AGC and AEC enable.
		{0x17, 0x14}//55/ COM8 : Fast AGC Algorithm, Unlimited Step Size , Banding Filter ON, AGC and AEC enable.
*/
		//
		//{0xb0, 0x84},
		//{0x13, 0xE5},
		//{0x56, 0x50},
		//{0x55, 0xb0}
//		{0x12, 0x0c}, //COM7 DVE: RGB mode, color bar disabled, QCIF (= 176x144 pixels)
//		{0x8c, 0x00},//2 / RGB444: 0b [7-2] Reserved, [1] RGB444 [0] XR GB(0) , RG BX(1)
//		{0x04, 0x00},//3 / COM1 : CCIR656 Format Disable
//		{0x40, 0x10}, //COM15 DVE: RGB565 mode
//		{15,0b100000},
//		{0x14, 0x18}//5 / COM9 : 4x gain ceiling
		//{0x11, 0x01},//6 / CLKRC : (was 3f) Internal Clock, [000000] to [111111] , 16000000 / ( [111111]+ 1 ) = 250000 DVE: deze deelt mijn MCO out van 36MHz [b0..5 + 1]
		//{0x6b, 0x4a},//72/ DBLV : PPL Control, PLL clk x 4, together with CLKRC it defines the frame rate
		//
};

#ifdef normal

//interrupts

void hrefrising();
void hreffalling();
void vsyncrising();
void startdmaline();


/* this is for the memory size
    BLOCKSIZE is the length of an image line which is one block
    MAXNUMBER is the amaount of buffer that is available
*/

#define BLOCKSIZE WIDTH*2 //*2 because of 16 bit image data
#define MAXNUMBER LINEBUFFERSIZE*2 //*2 because we can... :) should not be bigger than 120

uint8_t memmory[MAXNUMBER][BLOCKSIZE]; // the pysical buffer for image data
uint8_t memmory_state[MAXNUMBER]; // information about the state of image buffer blocks
int8_t memmory_index_to_linenumber[HEIGHT]; //-1 for empty 0-125 for index in memmory (index = linenumber)

#define MEMMORY_STATE_EMPTY  0  //block is empty
#define MEMMORY_STATE_FULL   1  //block is full but not used by any encoder
#define MEMMORY_STATE_LOCKED 2  //block is used by encoder
#define MEMMORY_STATE_LOCKED_BY_DRIVER 3 //just one block can be locked by the driver!!!

volatile uint16_t linecount; // realtime count for image data
volatile uint16_t lastrequestetline; // the line that was requestet with get the last time


//interrupt only
volatile uint16_t fps_counter;
volatile uint16_t fps_timer;

//controll objects for dma and sccb interface

DMAChannel dma;



void printdebug()
{
}

void dummyprocessor()
{
    int buffer[HEIGHT];
    for(int x = 0; x < HEIGHT;x++)
    	buffer[x]=0;
    	int r = 0;
    for(int x = 0; x < HEIGHT;x++)
    {
        r=(r+100)%HEIGHT;
        getline(r);
        delay(1);
        printdebug();
        freeline(r);
    }
	Serial.println("");

}

int changeregister(uint8_t adress,uint8_t data)
{
	sccb sccb_controll;
	sccb_controll.sccb_writebyte(adress, data);
    {
	  return 1;
    }
    return 0;
}

double getfps()
{
	return (double)(fps_counter*1000)/(double)(millis()-fps_timer);
}


void init_mem()
{
  linecount =0;
  fps_counter=0;
  fps_timer=millis();
  lastrequestetline = 0;
  for(int x = 0; x < MAXNUMBER;x++){
        memmory_state[x] =  MEMMORY_STATE_EMPTY;
  }
  for(int x = 0; x < HEIGHT; x++)
    memmory_index_to_linenumber[x]=-1;
}


void cleanup()//not very nice coding but it should work
{
    for(int l = 0; l < HEIGHT;l++)
    {
        if(memmory_index_to_linenumber[l]>=0)
            if(memmory_state[memmory_index_to_linenumber[l]]==MEMMORY_STATE_FULL)
            {
                memmory_state[memmory_index_to_linenumber[l]]=MEMMORY_STATE_EMPTY;
                memmory_index_to_linenumber[l]=-1;
            }
    }
}


uint8_t get_a_free_blocknumber()
{
    for(int x = 0; x < MAXNUMBER;x++) // check for empty blocks
        if(memmory_state[x]==MEMMORY_STATE_EMPTY)
        {
            return x;
        }
    for(int x = 0; x < MAXNUMBER;x++) // if no empty blocks are found... try to overwrite existing ones that are NOT used
        if(memmory_state[x]==MEMMORY_STATE_FULL)
        {
            memmory_state[x]= MEMMORY_STATE_EMPTY;
            for(int l = 0; l < HEIGHT;l++) // clear the link in memmory_index
            {
                if(memmory_index_to_linenumber[l]==x){
                    memmory_index_to_linenumber[l]=-1;
                    break;
                }
            }
            return x;
        }
    ERROR_MESSAGE("[ov7670] no free block has been found!!! \r\n");
    return 0;

}

uint8_t is_block_already_existing(uint16_t line_number)
{
    if(memmory_index_to_linenumber[line_number]>=0)
        return 1;
    return 0;
}




void init_ov7670()
{
init_mem();
  pinMode(HREF_PIN, INPUT);
  pinMode(VSYNC_PIN, INPUT);
  pinMode(XCLK_PIN, OUTPUT);
  pinMode(PCLK_PIN,INPUT);
  pinMode(DO_PIN, INPUT);
  pinMode(D1_PIN, INPUT);
  pinMode(D2_PIN, INPUT);
  pinMode(D3_PIN, INPUT);
  pinMode(D4_PIN, INPUT);
  pinMode(D5_PIN, INPUT);
  pinMode(D6_PIN, INPUT);
  pinMode(D7_PIN, INPUT);

#ifdef DEBUGPIN
  pinMode(DEBUG_PIN,OUTPUT);
#endif


  analogWriteFrequency(XCLK_PIN,XCLK); // generate clock for camera
  analogWrite(XCLK_PIN, 64);

  delay(100); //wait for clock to start

  for(int x = 0; x < SIZEOFREG;x++)
  {
	  changeregister(OV7725_Reg[x][0],OV7725_Reg[x][1]);
  }

  delay(400); //wait for register change


 //this is for the PCLK DMA trigger thing...
  GPIOE_PDDR &= ~0x02; // gpio input pin 22 on teensy pin 2 on cpu
  PORTE_PCR1 = (1 << 16) | (1 << 8); // DMA request o und mux gpio
  PORTE_PCR1 &= ~1; //gpio input




  dma = DMAChannel();
  dma.disable(); // just to be sure nothing bad happens...

  dma.triggerAtHardwareEvent(DMAMUX_SOURCE_PORTE); //PCLK is on PORTE


  dma.disableOnCompletion();    //not nessasary because we disable on line end trigger anyway
  dma.source((volatile uint8_t&)GPIOD_PDIR); //GPIO PORTD is the source for image data
  /* this is for xbee dma driver
#ifdef SEND_WIFI
  DMAPriorityOrder(dma,  *DMASPI0.getrxChannel(), *DMASPI0.gettxChannel());
#endif*/


  NVIC_SET_PRIORITY(IRQ_PORTA, PRIORITY_OV7670); // high priority for camera interrupts!
  NVIC_SET_PRIORITY(IRQ_PORTC, PRIORITY_OV7670);// high priority for camera interrupts!


  attachInterrupt (VSYNC_PIN, vsyncrising, FALLING); //start listening for new frame interrupts
}



void startimageandfreeram()
{
  stopimageandfreeram();
  attachInterrupt (VSYNC_PIN, vsyncrising, FALLING);
}



uint8_t* getline(int linenumber)
{

  lastrequestetline = linenumber; // update last requestet line(could cause problems with intterrupts)
  int time = millis(); //für den timeout

    if(linenumber>HEIGHT || linenumber < 0) // check for out of border
    {
        ERROR_MESSAGE("[ov7670] out of border get request\r\n");
        return 0;
    }

    int index = -1; // the index in memmory

  while(1)
  {
      index = memmory_index_to_linenumber[linenumber];//(could cause problems with intterrupts)
      if(index >=0)
      {
          switch(memmory_state[index]) //(could cause problems with intterrupts)
          {
              case MEMMORY_STATE_FULL:
                memmory_state[index] = MEMMORY_STATE_LOCKED; //(could cause problems with intterrupts)
                return (uint8_t*)memmory[index]; //(could cause problems with intterrupts)
              case MEMMORY_STATE_LOCKED:
                ERROR_MESSAGE("[ov7670] try to request locked block... YES that is bad!\r\n");
                return NULL;
                default:
              break;
          }
      }


	  delay(1);//WHY!!!!

      if(millis()-time>OV7670_TIMEOUT) // timeout
     {
    	ERROR_MESSAGE("[ov7670] THIS IS NO GOOD!!! camera timed out!!!!\r\n");
    	Serial.print("memmory_state: ");
    	for(int x = 0; x < MAXNUMBER; x++)
              {
                  Serial.print(memmory_state[x]);
                  Serial.print(",");
              }
                    Serial.print("\r\n mitln: ");
                for(int x = 0; x < HEIGHT; x++)
              {
                  Serial.print(memmory_index_to_linenumber[x]);
                  Serial.print(",");
                  if(HEIGHT/2==x)
                    Serial.println("\\");
              }
                    Serial.print("\r\n lastreq: ");
                    Serial.println(lastrequestetline);


                    Serial.print("lineindex for linenumber: ");
                    Serial.println(memmory_index_to_linenumber[linenumber]);

                    Serial.print("memmory state: ");
                    Serial.println(memmory_state[memmory_index_to_linenumber[linenumber]]);

    	return 0;
     }
  }
}

void freeline(int line)
{
  if(line>HEIGHT ||line < 0)
    {
        ERROR_MESSAGE("[ov7670] out of border free request\r\n");
        return;
    }

    int index = memmory_index_to_linenumber[line];
    if(index <0){
        ERROR_MESSAGE("[ov7670] this line is not free\r\n");
        return;
    }
    if(memmory_state[index]!=MEMMORY_STATE_LOCKED){
        ERROR_MESSAGE("[ov7670] this line");
        Serial.print(line);
        ERROR_MESSAGE(" on index ");
        Serial.print(index);
        ERROR_MESSAGE(" state: ");
        Serial.print(memmory_state[index]);
        ERROR_MESSAGE(" was not locked\r\n");
        return;
    }
    memmory_state[index]=MEMMORY_STATE_EMPTY;
    memmory_index_to_linenumber[line]=-1;

}

void stopimageandfreeram()
{
	ERROR_MESSAGE("[ov7670] stop image and free ram should not be used that often\r\n");
  detachInterrupt(HREF_PIN);
  detachInterrupt(VSYNC_PIN);
  dma.disable();
    init_mem();
}

#define BACK    LINEBUFFERSIZE-2
#define FRONT   2

void startdmaline()
{
    if((((lastrequestetline+BACK)>linecount && (lastrequestetline-FRONT)<linecount) ||
        ((lastrequestetline-FRONT)<0 && linecount>(HEIGHT+lastrequestetline-FRONT)) ||
        ((lastrequestetline+BACK)>HEIGHT && linecount<(BACK+lastrequestetline-HEIGHT))) &&
        (linecount < HEIGHT)) // block needed
    {
     if(memmory_index_to_linenumber[linecount]>=0)          //check if line has already been read
        return;
      uint8_t index = get_a_free_blocknumber();             //get a free block
      memmory_index_to_linenumber[linecount]=index;         //set index in linecount buffer;
      memmory_state[index]=MEMMORY_STATE_LOCKED_BY_DRIVER;   // lock the buffer
      dma.destinationBuffer(memmory[index],DMA_DATALENGTH);  // set the buffer for DMA
      attachInterrupt (HREF_PIN, hrefrising, RISING); // if not attached DMA will not be enabled
    }
 /*   else //block not needed
    {
        int index = memmory_index_to_linenumber[linecount];
        if(index>=0) //check if block can be removed
        {
            if(memmory_state[index]==MEMMORY_STATE_FULL)
            {
               memmory_state[index]=MEMMORY_STATE_EMPTY;
                memmory_index_to_linenumber[linecount]=-1;
            }
        }
    }*///thats the better way but its not working... need to sleep over it...
}



void vsyncrising() //end of frame and start of frame
{
	STARTWORKING;

    cleanup(); // BAD CODING!!!!
	fps_counter++; // frame counter...
    dma.disable();// just to be sure it is off
    linecount = 0;// prepare for next line
    startdmaline();// prepare for next line


	STOPWORKING;
}

void hrefrising() //start of line (is only calle when the line needs to be read!)
{
	STARTWORKING;

      //while (!GPIOD_PDIR);
      dma.enable(); // enable DMA
      attachInterrupt (HREF_PIN, hreffalling, FALLING); // attach end of line trigger

	STOPWORKING;
}


void hreffalling()//end of line
{
	STARTWORKING;

        int index = memmory_index_to_linenumber[linecount];
        if(index >= 0)
            if(memmory_state[index]==MEMMORY_STATE_LOCKED_BY_DRIVER)
                memmory_state[index]=MEMMORY_STATE_FULL;
        dma.disable(); // just to be sure it is off
        linecount++;  // prepare for next line
        startdmaline();// prepare for next line

	STOPWORKING;
}


#endif




