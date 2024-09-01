#ifndef SETPOINT_H
#define SETPOINT_H

#include <stdlib.h>
#include <stdint.h>

//initalise setpoints
void initSetpoint(void);
/*
This function increments the altitude when the UP button is called
*/
void incrementAltitudeSetpoint(void);

/*
This function decrements the altitude when the down button is called
*/
void decrementAltitudsetpoint(void);

/*
Increment the yaw count which will turn the helicopter CW when the RIGHT button is pressed
*/
void incrementYawSetpoint(void);
/*
Increment the yaw count which will turn the helicopter CCW when the LEFT button is pressed
*/
void decrementYawSetpoint(void);

/*
Returns the altitude setpoint
*/
int16_t getAltitudeSetpoint(void);

/*
Return the yaw Setpoint.
*/
int16_t getYawSetpoint(void);

/*
Set the altitude setpoint.
*/
void setAltitude(int32_t);

/*
Set the yaw setpoint.
*/
void setYaw(int16_t);



#endif
