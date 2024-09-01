/*************************************************************************************
setpoint.c - Module to calculate the setpoints for which the PID control will use
             to move to the desired position based on user input to change 
             altitude/yaw.
             
             This module should only be called if there is a button press that 
             occurs. If there is no button press, then the setpoint used in the PID
             controller should not change.
Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 7th of May, 2024.
***************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>

#include "Altitude.h"
#include "setpoint.h"
#include "Yaw.h"


static const int16_t deltaAltitude = (10 * ONE_VOLT_BITS_VAL) / 100; // Change to altitude is the UP or DOWN button are pressed.
static const int16_t deltaYaw = (15 * QUAD_NUMBER_OF_TEETH) / TOTAL_DEGREES; // Change to yaw if the RIGHT or LEFT button are pressed.

int16_t altitudeSetpoint; // Altitude setpoint that helicopter desires to reach.
int16_t yawSetpoint; // Yaw setpoint that helicopter desires to rotate too.
int16_t MAX_ALTITUDE ;
int16_t MIN_ALTITUDE;
bool upPressed = false; // Flag to indicate whether UP button press has occured.
bool downPressed = false; // Flag to indicate whether DOWN button press has occured.
bool rightPressed = false; // Flag to indicate whether RIGHT button press has occured.
bool leftPressed = false; // Flag to indicate whether LEFT button press has occured.

bool increaseHeight = true; // Flag to indicate whether the helicopter is moving up.


void initSetpoint(void)
{
    MIN_ALTITUDE = getGround();
    MAX_ALTITUDE = getTop();
    altitudeSetpoint = MIN_ALTITUDE;
    yawSetpoint = 0;
}

void incrementAltitudeSetpoint(void)
{
    int32_t currentAltitude = getAverageADCVal();
    // Calculate new required altitude setpoint
    altitudeSetpoint = altitudeSetpoint - deltaAltitude; // subtract because the altitude increases with nega

    // Check to see if the new altitude setpoint calculated is greater than the maximum altitude
    if (altitudeSetpoint < MAX_ALTITUDE) {
        altitudeSetpoint = MAX_ALTITUDE;
    } 
}

void decrementAltitudsetpoint(void)
{
    int32_t currentAltitude = getAverageADCVal();
    // Calculate new required altitude setpoint
    altitudeSetpoint = altitudeSetpoint + deltaAltitude;
    
    // Check to see if the new altitude setpoint calculated is less than the minimum altitude
    if (altitudeSetpoint > MIN_ALTITUDE) {
        altitudeSetpoint = MIN_ALTITUDE;
    } 
}

void incrementYawSetpoint(void)
{
    // Increment the yaw by a constant
    yawSetpoint = yawSetpoint + deltaYaw;
    if (yawSetpoint >= QUAD_NUMBER_OF_TEETH) {
        yawSetpoint = yawSetpoint - QUAD_NUMBER_OF_TEETH;
    }
}

void decrementYawSetpoint(void)
{
    // Decrement the yaw by a constant
    yawSetpoint = yawSetpoint - deltaYaw;
    if (yawSetpoint < 0) {
        yawSetpoint = QUAD_NUMBER_OF_TEETH + yawSetpoint;
    }
}

int16_t getAltitudeSetpoint(void)
{
    // Return altitude setpoint
    return altitudeSetpoint;
}

int16_t getYawSetpoint(void)
{
    // Return yaw setpoint
    return yawSetpoint;
}

void setAltitude(int32_t altSet) {
    altitudeSetpoint = altSet;
}

void setYaw(int16_t yawSet)
{
    yawSetpoint = yawSet;
}
