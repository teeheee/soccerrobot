
#include "Arduino.h"

#define mainloop
//#define watchdog
//#define kameratestloop
//#define seriallooptoxbee
//#define wifitestloop
//#define wifidmatestloop

#ifdef mainloop

#include "aktuators.h"
#include "XBee.h"
#include "wifi.h"
#include "config.h"
#include "SPI.h"
#include "led.h"
#include "akku.h"
#include "sensors.h"
#include "ov7670.h"
#include "jpeg.h"
#include "wifistream.h"

#define PREINIT 	0
#define SENSORINIT  1
#define WIFIINIT	2
#define MAINLOOP    3
#define ERROR   	4
#define USBCONFIG   5
#define MOTORINIT   6
#define KAMERAINIT  7


uint8_t robot_state = 0; // current state of the robot
const char* errortext; // stores last errormessage
struct config c; //configuration of robot. It is loaded from eeprom

/***WATCHDOG*****/

#ifdef WATCHDOG
IntervalTimer watchdogtimer;


#ifdef __cplusplus
extern "C" {
#endif
void startup_early_hook() {

	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH = (WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN | WDOG_STCTRLH_STOPEN); // Enable WDG
	WDOG_TOVALL = 2000; // The next 2 lines sets the time-out value. This is the value that the watchdog timer compa
	WDOG_TOVALH = 0;
	WDOG_PRESC = 0; // prescaler
}
#ifdef __cplusplus
}
#endif

void WatchdogReset()
{
// use the following 4 lines to kick the dog
noInterrupts();
WDOG_REFRESH = 0xA602;
WDOG_REFRESH = 0xB480;
interrupts();
// if you don't refresh the watchdog timer before it runs out, the system will be rebooted
delay(1); // the smallest delay needed between each refresh is 1ms. anything faster and it will also reboot.

}
#endif
/**********************/

void serialEvent() //Serial event
{
	if (Serial.available() != 0 && robot_state != PREINIT)
		robot_state = USBCONFIG;
}

void errormessage(const char* text) { //errorhandling and display. Could be better
	errortext = text;
	robot_state = ERROR;
	motors(0, 0);
	wifi_reset();
}

int setupwifi() { //setup network connection. could still be faster

	Serial.println(F("[wifi] try to init"));
	int ret = wifi_init(c.tcp_or_ip);
	if (ret > 0) {
		errormessage("[wifi] wifi init error");
		return ret;
	} else if (ret < 0)
		Serial.println(F("[wifi] wifi already initialised"));

	Serial.println(F("[wifi] try to connect to AP"));
	ret = wifi_connecttoAp("welcome");
	if (ret > 0) {
		errormessage("[wifi] wifi AP error");
		return ret;
	} else if (ret < 0)
		Serial.println(F("[wifi] AP already initialised"));

	Serial.print(F("[wifi] try configure server with ip: "));
	printip(c.ip);
	Serial.println(F(""));
	ret = wifi_connecttoServer(c.ip, (uint16_t) c.port);
	if (ret > 0) {
		errormessage("[wifi] wifi server error");
		return ret;
	} else if (ret < 0)
		Serial.println(F("[wifi] server already initialised"));
	int timeout = millis();
	while (wifi_isready()) {
		delay(100);
		unsigned char stat = betterstatus();
	    if (stat == 0){
						Serial.println(F("[wifi] connected"));
						return 0;
	    }
		if (millis() - timeout > 10000) {

			errormessage("[wifi] error!!!!!!");
			return 1;
		}
	}
	Serial.println(F("[wifi] connected"));
	return 0;
}

void config() { //config mode
	flushinput();
	while (1) {
		clearscreen();
		Serial.println(F("In config mode. Choose:"));
		Serial.println(F("1: change ip"));
		Serial.println(F("2: run motor test"));
		Serial.println(F("3: run camera test"));
		Serial.println(F("4: run wifi test"));
		Serial.println(F("5: run sensor test"));
		Serial.println(F("6: change udp tcp"));
		Serial.println(F("7: change motor direction"));
		Serial.println(F("8: exit"));
		Serial.println(F("9: xbee factory reset"));
		Serial.println(F("10: xbee torture test"));
		Serial.println(F("11: wifistream test"));
		Serial.println(F("12: change ov7670 register"));
		Serial.println(F("13: change motor minimal speed"));
		Serial.println(F("14: set IR Goal value"));
		switch (getint()) {
		case 14: //set IR Goal value
			clearscreen();
			flushinput();
			{
			Serial.println(F("Turn the Robot to the Goal!"));
			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			long value = 0;
			for(int i = 0; i < 100;i++){
				uint32_t t = 0;
				t = read_ir();
				while(t == 0)
				{
					t = read_ir();
					delay(100);
					if(Serial.available()>0)
					{
						i=100;
						break;
					}
				}
				Serial.println(t);
				value+=t;
			}
			value /= 100;
			value -= 100;
			c.irtorwert = value;
			Serial.print(F("done.New Goal Value: "));
			Serial.println(value);
			saveeeprom(c);
			}
			flushinput();
			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			break;
		case 13: //ov7670 register change
					flushinput();
					{
						Serial.println(F("press Enter to continue..."));
						waitforbutton();
						flushinput();
						int first = getminimalspeed(0);
						Serial.println(F("press Enter to continue..."));
						waitforbutton();
						flushinput();
						int second = getminimalspeed(1);
						setminimalspeed(first,second);
						c.motorminspeedleft=first;
						c.motorminspeedright=second;
						Serial.print(F("leftmotor: "));
						Serial.print(first);
						Serial.print(F(" rightmotor: "));
						Serial.println(second);
						saveeeprom(c);
					}
					flushinput();
					Serial.println(F("press Enter to continue..."));
					waitforbutton();
					break;
		case 12: //ov7670 register change
			flushinput();
			{
				Serial.println(F("this function is forbidden in normal use"));
				/*Serial.println(F("register"));
				uint8_t a = getint();
				Serial.println(F("data"));
				uint8_t b = getint();
				changeregister(a,b);*/
			}
			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 1: //ip config
			flushinput();
			Serial.println(F(""));
			Serial.print(F("please enter ip [default = "));
			printip(c.ip);
			Serial.print(F("]:"));
			readip(c.ip);
			Serial.print(F("new ip is: "));
			printip(c.ip);
			saveeeprom(c);
			Serial.println(F(""));
			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 2: //motor test
			Serial.println(F("WATCH THE ROBOT!!!!"));
			Serial.println(F("3"));
			delay(1000);
			Serial.println(F("2"));
			delay(1000);
			Serial.println(F("1"));
			delay(1000);

			Serial.println(F("left backward->forward"));
			for (int x = -100; x < 100; x++) {
				motors(x, 0);
				delay(20);
			}
			Serial.println(F("right backward->forward"));
			for (int x = -100; x < 100; x++) {
				motors(0, x);
				delay(20);
			}
			Serial.println(F("both in same direction"));
			for (int x = -100; x < 100; x++) {
				motors(x, x);
				delay(20);
			}
			Serial.println(F("kick"));
			motors(0, 0);
			kick();
			Serial.println(F("DONE"));
			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 3: //video test
#ifdef VIDEO
			flushinput();
			init_ov7670();
			while (Serial.available() == 0)
				dummyprocessor();
			flushinput();
			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
#endif
			break;
		case 4: //wifi test
			if (setupwifi()) {
				Serial.println(F("wifi setup error!!!!!!!"));
				Serial.println(F("waiting for reset...."));
				delay(3000);
			}
			Serial.println(F("--------xbee config-------"));
			print_xbee_config();
			Serial.println(F("--------start test-------"));
			delay(1000);
			for (int x = 1; x < 2000; x += x) {
				Serial.print(F("packsize: "));
				Serial.print(x, DEC);
				Serial.print(F(" speed: "));
				int i = speedtest(x, 10);
				if (i < 0)
					break;
				Serial.print(i, DEC);
				Serial.println(F(" kB/s"));
			}
			delay(1000);
			Serial.println(F("--------stop test-------"));
			//dns_lookup("welcome.uni-ulm.de"));

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 5: // sensor test
			flushinput();
			Serial.println(F(" "));
			while (Serial.available() == 0) {
				clearline();
				Serial.print(F("Akku: "));
				Serial.print(akku_percent());
				Serial.print(F("% Zd: "));
				Serial.print(akku_zellendif());
				Serial.print(F("V Z1: "));
				Serial.print(akku_zelle1());
				Serial.print(F("V Z2: "));
				Serial.print(akku_zelle2());
				Serial.print(F("V IR: "));
				Serial.print(read_ir());
				Serial.println(F(" "));

				delay(300);
			}

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 6: //tcp udp umstellen
			Serial.println(F("TCP[1],UDP[2]:"));
			c.tcp_or_ip = getint();
			saveeeprom(c);
			wifi_reset();

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 7: //change motordir
			Serial.println(F("Motorlinks [0 normal, 1 invertiert]:"));
			c.motordirleft = getint();
			Serial.println(F("Motorrechts [0 normal, 1 invertiert]:"));
			c.motordirright = getint();
			saveeeprom(c);
			setdirection(c.motordirleft, c.motordirright);

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			return;
		case 8: //restart
			Serial.println(F("restart..."));
			flushinput();
			return;
		case 9:

			Serial.println(F("this function is forbidden in normal use"));
			/*
			Serial.println(F("xbee factory rest nothing will be working anymore"));
			factory_reset();*/

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		case 10:
			flushinput();{
			int timerstart = millis();
			while (Serial.available() == 0) {
				Serial.print(F("packsize: "));
				Serial.print(1400, DEC);
				Serial.print(F(" speed: "));
				int lasttime = millis();
				int i = speedtest(1400, 20);
				if (i < 0) {
					Serial.println(F(""));
					Serial.println(F("---error---"));
					Serial.print(F("errorcode: "));
					Serial.println(-i, DEC);
					betterstatus();
					Serial.print(F("time to failure: "));
					Serial.print(lasttime - timerstart, DEC);
					Serial.println(F("ms"));
					Serial.println(F("---------"));
					Serial.println(F(""));
					break;
				}
				Serial.print(i, DEC);
				Serial.println(F(" kBit/s"));
			}

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
			}
		case 11:
			flushinput();
			output_stream stream;
			while (Serial.available() == 0) {
				void* x = malloc(100);
				memset(x, 0x42, 100);
				stream.put_buf(x, 100);
				free(x);
				delay(10);
			}

			Serial.println(F("press Enter to continue..."));
			waitforbutton();
			flushinput();
			break;
		}

	}

}

void sendstatus(uint8_t* payload, int payloadlength) { //send status with optional payload

#ifdef SEND_WIFI
	int wstate = wifi_isready();
	if (!wstate) {
		struct status s;
		generatestatusmessage(s);
		int failed_counter = 0;
		ledOn(LED_RED);
		while (1) {
			int ret = wifi_senddata((uint8_t*) &s, sizeof(s));
			if (ret == 0)
				break;
			failed_counter++;
			if (failed_counter == 1) {
				ledOff(LED_RED);
				Serial.print(F("[main] errorcode:"));
				Serial.println(ret);
				betterstatus();
				errormessage("send error");
				return;
			} else
				Serial.println(F("[main] try again.."));
		}
		ledOff(LED_RED);
	} else {
		errormessage("wifi not ready");
	}
#endif
}

void setup() {

#ifdef WATCHDOG
	watchdogtimer.begin(WatchdogReset,1000000);
#endif
	ledinit();
	ledOn(LED_RED);
	ledBlinkOn(LED_GREEN);
	Serial.begin(9600);
	//waitforbutton();
	Serial.setTimeout(10000000);
	robot_state = PREINIT;
	Serial.println(F("start.."));
}

void loop() {
	switch (robot_state) {
	case PREINIT:
		ledOff(LED_RED);
		ledBlinkOn(LED_GREEN);
		init_akku();
		Serial.print(F("Akkustand: "));
		Serial.print(akku_percent());
		Serial.print(F("%"));
		Serial.print(F("Akkudiff: "));
		Serial.print(akku_zellendif());
		Serial.println(F("V"));
		loadeeprom(&c);

		robot_state = MOTORINIT;
		break;
	case MOTORINIT:
		ledOff(LED_RED);
		ledBlinkOn(LED_GREEN);
#ifdef MOTORS
		Serial.println(F("[main] setup aktuators.."));
		initaktuators();
		setdirection(c.motordirleft, c.motordirright);
		setminimalspeed(c.motorminspeedleft, c.motorminspeedright);
#endif
		robot_state = SENSORINIT;
		break;
	case SENSORINIT:
		ledOff(LED_RED);
		ledBlinkOn(LED_GREEN);
		Serial.println(F("[main] setup sensors.."));
		init_sensors();
		robot_state = KAMERAINIT;
		break;
	case KAMERAINIT:
		ledOff(LED_RED);
		ledBlinkOn(LED_GREEN);
#ifdef VIDEO
		Serial.println(F("[main] setup camera..."));
		init_ov7670();
#endif
		robot_state = WIFIINIT;
		break;
	case WIFIINIT:
		ledOff(LED_RED);
		ledBlinkOn(LED_GREEN);
#ifdef SEND_WIFI
		Serial.println(F("[main] setup wifi.."));
		if (setupwifi())
			break;
#endif
		Serial.println(F("[main] starting main loop"));
		Serial.println(F(""));
		robot_state = MAINLOOP;
		delay(500); // not to much stuff for xbee
		break;

	case MAINLOOP:
		ledOn(LED_GREEN);
		ledOff(LED_RED);

		//***test****//
		if(digitalRead(XBEE_ATN) == 0)
			attn_interrupt();
		//*****/

		if (getcamerastate()) {
			static int framesize = 0;
			static int framecounter = 0;
			static int firstmillis = millis();
			static int lastmillis = millis();
			framecounter++;
			if (millis() - lastmillis > 1000) {
				clearline();
				Serial.print(F("[main] send fps: "));
				double fps = (double)(framecounter*1000)/(double)(millis()-firstmillis);
				double rate = (double)(framesize*8)/(double)(millis()-firstmillis);
				Serial.print(fps);
				Serial.print(F(" raw data: "));
				Serial.print(HEIGHT*8*WIDTH*fps/2000);
				Serial.print(F(" kbit/s"));
				Serial.print(F(" jpeg data: "));
				Serial.print(rate);
				Serial.print(F(" kbit/s"));
				Serial.print(F(" last message: "));
				int t = get_last_message_time();
				Serial.println(t);
			//	if(t>1000)
			//		motors(0,0);
				lastmillis = millis();
			}
			static output_stream stream;
			int s=encodeimage(stream);
			/*
				Serial.print(s,DEC);
				Serial.print(F(","));
			*/
			if(!s)
				errormessage("[main] some jpeg error");
			framesize+=s;
		} else {
			clearline();
			Serial.print(F("[main] send status with no picture"));
			Serial.print(F(" | last message [ms]: "));
			int t = get_last_message_time();
			Serial.println(t);
			sendstatus(0, 0);
			//if(t>1000)
			//	motors(0,0);
			delay(200);
		}

		break;

	case ERROR:
		ledOff(LED_GREEN);
		ledBlinkOn(LED_RED);
		ERROR_MESSAGE("[error] ");
		ERROR_MESSAGE(errortext);
		Serial.println(F(""));
		delay(1000);
		robot_state = WIFIINIT;
		break;

	case USBCONFIG:
		ledBlinkOn(LED_RED);
		ledBlinkOn(LED_GREEN);
		config();
		robot_state = PREINIT;
		break;
	default:
		break;
	}
}
#endif

#ifdef watchdog

#include "Arduino.h"
#include "aktuators.h"
#include "XBee.h"
#include "wifi.h"
#include "config.h"
#include "SPI.h"
#include "led.h"
#include "akku.h"
#include "sensors.h"
#include "ov7670.h"
#include "jpeg.h"
#include "wifistream.h"


#ifdef __cplusplus
extern "C" {
#endif
void startup_early_hook() {

	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH = (WDOG_STCTRLH_ALLOWUPDATE | WDOG_STCTRLH_WDOGEN | WDOG_STCTRLH_WAITEN | WDOG_STCTRLH_STOPEN); // Enable WDG
	WDOG_TOVALL = 2000; // The next 2 lines sets the time-out value. This is the value that the watchdog timer compa
	WDOG_TOVALH = 0;
	WDOG_PRESC = 0; // prescaler
}
#ifdef __cplusplus
}
#endif

void setup()
{
	Serial.begin(9600);
}

void loop()
{
	static int x = 0;
	x++;
	WatchdogReset();
	delay(100*x);
	Serial.print(F("X: "));
	Serial.println(x);
}

#endif

#ifdef kameratestloop

#include "Arduino.h"
#include "aktuators.h"
#include "XBee.h"
#include "wifi.h"
#include "config.h"
#include "SPI.h"
#include "led.h"
#include "akku.h"
#include "sensors.h"
#include "ov7670.h"
#include "jpeg.h"
#include "wifi.h"
//#include "wifistream.h"



//#define dummy
//#define direktusb
#define jpegusb




void serialEvent() //Serial event
{
	flushinput();
	waitforbutton();
}




void setup()
{
	ledinit();
	ledBlinkOn(LED_RED);
	Serial.begin(9600);
#ifdef dummy
	waitforbutton();
#endif
	Serial.println(F("start"));
    init_sensors();
    initaktuators();
    init_ov7670();
    ledBlinkOff(LED_RED);
    ledOn(LED_GREEN);
}

void loop()
{
	/*static int framecounter = 0;
	static int lastmillis = millis();
	framecounter++;
	if (millis() - lastmillis > 1000) {
		Serial.print(F("[main] fps: "));
		Serial.println(framecounter, DEC);
		framecounter = 0;
		lastmillis = millis();
	}*/


#ifdef dummy
	dummyprocessor();
	//delay(1000);
#endif

#ifdef direktusb
	Serial.print(F("FFFFFFFFFF"));
	for(int x = 0; x<HEIGHT;x++)
	{
		uint8_t* data = (uint8_t*) getline(x);
		for(int i = 1;i<WIDTH;i+=2)
		Serial.write(data[i]);
		//Serial.print(data[i],HEX);
		freeline(x);
	}
#endif

#ifdef jpegusb
	static output_stream stream;
	encodeimage(stream);
#endif

}

#endif

#ifdef seriallooptoxbee

#include "Arduino.h"
#include "aktuators.h"
#include "XBee.h"
#include "wifi.h"
#include "config.h"
#include "SPI.h"
#include "led.h"
#include "akku.h"
#include "sensors.h"
#include "ov7670.h"
#include "jpeg.h"
#include "wifistream.h"

void setup()
{
	Serial.begin(9600);
	Serial1.begin(9600);

}

void loop()
{

	if(Serial.available()>0)
	{
		Serial1.write(Serial.read());
	}
	if(Serial1.available()>0)
	{
		Serial.write(Serial1.read());
	}
}

#endif

#ifdef wifitestloop

#include "wifi.h"
#include "wifistream.h"
#include "config.h"
#include "led.h"
unsigned char ip_alex[] = { 134, 60, 135, 29 }; //ich

int setupwifi() { //setup network connection. could be faster
	//if(betterstatus()==0)
	//	return 0;

	Serial.println(F("[wifi] try to init"));
	int ret = wifi_init(UDP);
	if (ret > 0) {
//		errormessage(F("[wifi] wifi init error"));
		return ret;
	} else if (ret < 0)
		Serial.println(F("[wifi] wifi already initialised"));

	Serial.println(F("[wifi] try to connect to AP"));
	ret = wifi_connecttoAp("welcome"));
	if (ret > 0) {
	//	errormessage(F("[wifi] wifi AP error"));
		return ret;
	} else if (ret < 0)
		Serial.println(F("[wifi] AP already initialised"));

	Serial.print(F("[wifi] try configure server with ip: "));
//	printip(c.ip);
	Serial.println(F(""));
	ret = wifi_connecttoServer(ip_alex, (uint16_t)44044);
	if (ret > 0) {
//		errormessage(F("[wifi] wifi server error"));
		return ret;
	} else if (ret < 0)
		Serial.println(F("[wifi] server already initialised"));
	int timeout = millis();
	while (wifi_isready()) {
		delay(100);
		unsigned char stat = betterstatus();
	    if (stat == 0){
						Serial.println(F("[wifi] connected"));
						return 0;
	    }
		if (millis() - timeout > 10000) {

//			errormessage(F("[wifi] error!!!!!!"));
			return 1;
		}
	}
	Serial.println(F("[wifi] connected"));
	return 0;
}


void setup()
{
	Serial.begin(9600);
	ledinit();
	waitforbutton();

}
//#define mallocdata
void loop()
{

while(1){
	while(Serial.available()>0)Serial.read();
	ledBlinkOn(LED_GREEN);
	while(setupwifi());

	delay(2000);
	ledBlinkOff(LED_GREEN);
	ledOn(LED_GREEN);
	unsigned int time = millis();
	double datasize = 0;
	volatile uint8_t* constdata;
	constdata = (uint8_t*) malloc(1400);;
	for(int i = 0; i < 1400;i++)
		constdata[i]=(uint8_t)random(0,0xff);

	while(Serial.available()==0)
	{
		static int x = 0;
		x++;
		int r = random(1,1400);
#ifdef mallocdata
		uint8_t* data = (uint8_t*) malloc(r);
		for(int i = 0; i < r;i++)
			data[i]=(uint8_t)random(0,0xff);
		//memset(data,0x42,r);
#endif
unsigned int lastmillis = millis();
		ledOn(LED_RED);

	//	unsigned int delaytime = millis();
	//	while(millis()-delaytime<r);

		//while(wifi_isready())delay(10);

		int i = 0;
		do{
#ifdef mallocdata
		i = wifi_senddata((unsigned char*)data,r);
#else
		i =  wifi_senddata((unsigned char*)constdata,r);
#endif
		if(i==3)
			Serial.println(F("----buffer voll-----"));
		}while(i==3);


		ledOff(LED_RED);

		if(millis()%20000==0)
		{

			Serial.print(F("running without error: "));
			Serial.print((lastmillis-time)/1000);
			Serial.println(F("s"));
			Serial.print(F("DATARATE: "));
		    Serial.print((int)( (double)datasize / ((double)(lastmillis-time) / (double)8000 ))/1000,DEC);
			Serial.println(F("kbit/s"));
		}

		if(i>0 || millis()-lastmillis>1000)
		{
			Serial.print(F("time until last send: "));
			Serial.print((lastmillis-time)/1000);
			Serial.println(F("s"));
			Serial.print(F("time until error detected: "));
			Serial.print((millis()-lastmillis)/1000);
			Serial.println(F("s"));
			Serial.print(F("DATARATE: "));
		    Serial.print((int)( (double)datasize / ((double)(lastmillis-time) / (double)8000 ))/1000,DEC);
			Serial.println(F("kbit/s"));
			Serial.print(F("lastpacketsize: "));
		    Serial.println(r);
			Serial.print(F("packagenumber: "));
			Serial.println(x);
			Serial.print(F("returncode: "));
			Serial.println(i,HEX);
			//print_xbee_config();
			//waitforbutton();
			delay(1000);
			datasize=0;
			time = millis();
			x=0;
		}
		else{
			//Serial.print(r);
			//Serial.print(F(","));
		}
		datasize+=r;
#ifdef mallocdata
		free(data);
#endif

	}
	wifi_reset();
 }
}

#endif

#ifdef wifidmatestloop

#include "DMAXBEE.h"
#include "config.h"
#include "led.h"
unsigned char ip_alex[] = { 134, 60, 135, 29 }; //ich

void setup()
{
	Serial.begin(9600);
	ledinit();
	waitforbutton();

}
void loop()
{

while(1){
	while(Serial.available()>0)Serial.read();
	ledBlinkOn(LED_GREEN);
	delay(2000);
	init_xbee(100);
	ledBlinkOff(LED_GREEN);
	ledOn(LED_GREEN);

	volatile char* constdata;
	constdata = (char*) malloc(1400);;
	for(int i = 0; i < 1400;i++)
		constdata[i]=(uint8_t)random(0,0xff);

	while(1)
	{

		waitforbutton();
		ledOn(LED_RED);
		send_data((char*)constdata,100);
		ledOff(LED_RED);
	}
	wifi_reset();
 }
}

#endif
