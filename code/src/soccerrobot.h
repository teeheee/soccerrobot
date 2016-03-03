// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef _soccerrobot_H_
#define _soccerrobot_H_
#include "Arduino.h"
//add your includes for the project soccerrobot here


//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif
void loop();
void setup();
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project soccerrobot here

void sendstatus(uint8_t* payload,int payloadlength);


//Do not add code below this line
#endif /* _soccerrobot_H_ */
