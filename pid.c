/*****************************************************************************
pid.c - Module to control the movement of the helicopter
        in both altitude and yaw as the the respective features 
        are incremented/decremented. The main and tail rotor both use PID
        algorithms. For the tail rotor, intergral control is only implimented
        if within a defined error range

Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 14th of May, 2024.
*********************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>


#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "OrbitOLED/lib_OrbitOled/OrbitOLED.h" 

#include "Altitude.h"
#include "Yaw.h"
#include "PWM.h"
#include "setpoint.h"
#include "kernel.h"
#include "pid.h"


/* 
global varibales for the gain values
*/
int32_t mainKp = 100000; 
int32_t mainKi = 100;
int32_t mainKd = 1000;
int32_t prevMainI = 0;

/*
The PID gains for tail rotor
*/
int32_t tailKp = 350000;
int32_t tailKi = 400;
int32_t tailKd = 0;
int32_t prevTailI = 0;
int8_t prevDuty = 0;

// Final output control value for Main Rotor and Tail Rotor.
int32_t mainControl = 0;
int32_t tailControl = 0;


//main rotor current error
int32_t mainCurrentError;
int32_t tailCurrentError;

//The previous values for the PID controller. Saved as volatile because changes often
static volatile int32_t prevYaw = 0;
static volatile int32_t prevAltitude;


void initPID(void)
{
    prevAltitude = getAverageADCVal();
    prevYaw = getYawCount();
}


/*
======================================================================================================================
MAIN ROTOR PID PROCESSING 
======================================================================================================================
*/
void mainGainClamp(void)
{
    // Gain clamp to check for integral windup.
    if (mainControl > MAX_MAIN_DUTY) {
        mainControl = MAX_MAIN_DUTY;
    } else if (mainControl < MIN_MAIN_DUTY) {
        mainControl = MIN_MAIN_DUTY;
    }
}

int8_t mainRotorController(void)
{
    // Gettting the current system information
    int32_t currentAltitude = getAverageADCVal();
<<<<<<< HEAD
    // Getting the altitude setpoint.
    int32_t altitudeSzetpoint = getAltitudeSetpoint();
    // Capturing the time when the function is called.
    //uint8_t main_dT = mainTimeChange();
=======
    int32_t altitudeSetpoint = getAltitudeSetpoint();
>>>>>>> 8aa787ad99843b00dba68880646f10ac5d45b7e7

    // Calculating the current error based on the difference between the altitude setpoint and current altitude
    mainCurrentError = currentAltitude - altitudeSetpoint;

    // The final claculated control output values for P, I, and D.
    int32_t main_P = mainKp * mainCurrentError;
    int32_t main_I = (mainKi * mainCurrentError * PID_PERIOD);

    // storing the previous value of the integral control
    main_I += prevMainI; 
<<<<<<< HEAD
    //P controll for simplistic setup
    int32_t mainD = (mainKd * (currentAltitude - prevAltitude)) / PID_PERIOD;
    tailDiv = main_P;
=======
    int32_t main_D = (mainKd * (currentAltitude - prevAltitude)) / PID_PERIOD;
>>>>>>> 8aa787ad99843b00dba68880646f10ac5d45b7e7
    
    // All the individual output control values added together.
    mainControl = ((main_P + main_I + main_D) / DIVISOR) + GRAVITY_CONSTANT;
    
    // Check there is no integral windup
    mainGainClamp();

    // Storing the previous values.
    prevMainI = main_I;
    prevAltitude = currentAltitude;

    return mainControl;
}


/*
======================================================================================================================
TAIL ROTOR PID PROCESSING 
*/
int8_t tailRotorController(void)
{
    // Get the current system information
    int32_t currentYaw = getYawCount();
    int32_t yawSetpoint = getYawSetpoint();

    // Finding the difference between the yaw setpoint and current angle displacement
    tailCurrentError = yawSetpoint - currentYaw;

    //Handle the loop condition so that error does not get larger than half of the count
    if (tailCurrentError > QUAD_NUMBER_OF_TEETH / 2)
    {
        tailCurrentError -= QUAD_NUMBER_OF_TEETH;
    } else if (tailCurrentError < -(QUAD_NUMBER_OF_TEETH / 2)) {
        tailCurrentError += QUAD_NUMBER_OF_TEETH;
    } 

    // Calculating individual tail control values for P, I, D.
    int32_t tailP = tailKp * tailCurrentError;
<<<<<<< HEAD
    int32_t tailI = tailKi * tailCurrentError * PID_PERIOD;
    tailI += prevTailI;
    int32_t tailD = (tailKd * (prevYaw - currentYaw)) / PID_PERIOD;  // think about loop condition
    //tailPrevError = (prevYaw - currentYaw);
=======
    int32_t tailI = 0;
    int32_t tailD = (tailKd * (prevYaw - currentYaw))/ PID_PERIOD;

    //Only apply intergral gains when within a certian error range. No windup
    if (abs(tailCurrentError) < ERROR_RANGE) {
        int32_t tailI = tailKi * tailCurrentError * PID_PERIOD;
        tailI += prevTailI;
    }

>>>>>>> 8aa787ad99843b00dba68880646f10ac5d45b7e7
    // All the individual output control values added together.
    tailControl = ((tailP + tailI + tailD) / DIVISOR) + (COUPLING_CONSTANT * getMainDuty())/COUPLING_DIVISOR;
    
    // Gain clamp to check for integral windup.
    if (tailControl > MAX_TAIL_DUTY) {
        tailControl = MAX_TAIL_DUTY;
    } else if (tailControl < MIN_TAIL_DUTY) {
        tailControl = MIN_TAIL_DUTY;
    }

    // Storing previous values to update PID
    prevYaw = currentYaw;
    prevTailI = tailI;

    return tailControl;
}

