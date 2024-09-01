#ifndef YAW_H
#define YAW_H

#include <stdint.h>
#include <stdbool.h>

#define QUAD_NUMBER_OF_TEETH   448
#define TOTAL_DEGREES  360

#define CHANNEL_A   GPIO_PIN_0
#define CHANNEL_B   GPIO_PIN_1
#define QUAD_NUMBER_OF_TEETH   448
#define TOTAL_DEGREES  360
#define YAW_PRIORITY 0
#define YAW_REF_PRIORITY 1

/*
get the state of the encoder based off current inputs
*/
uint8_t getState(bool A, bool B);

/*
yaw interupt handler for both encoder channels
*/
void yawIntHandler(void);

/*
initalise the quadrature encoder yaw 
*/
void initYaw(void);

/*
Function to call within the display module to get the yaw count value
*/
int16_t getYawCount(void);

/*
Converts the Yaw Count (based on the number of teeth) into degrees
*/
int32_t yawInDegrees(void);

/*
Interrupt hander for the yaw setpoint
*/
void yawRefIntHandler(void);

/*
reset the yaw reset flag
*/
void resetYawRefFlag(void);

/*
get the the yaw reset flag
*/
bool getYawRefFlag(void);



#endif 
