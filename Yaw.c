/*****************************************************************************
yaw.c - this modules controlls the yaw information. It will setup the pin change interrupts
        that keep track of the yaw position and yaw refference. Additonally, 
        it has getter functions for other modules to call that update yaw values

Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 14th of May, 2024.
*********************************************************************************/



#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "yaw.h"

static bool ChA;  
static bool ChB;
static bool yawRefFound = false;

static bool ChA_Previous;
static bool ChB_Previous;

uint8_t currentQuadState;
uint8_t previousQuadState;

static bool clockwise = false;
int16_t yawCount = 0; 

uint8_t getState(bool A, bool B)
{
    //a state calculation function should match binary channel number ie 01,10 etc
    if (!A && !B) { //00
        return 1;
    } else if (A && !B) { //10
        return 2;
    } else if (A && B) { //11
        return 3;
    } else if (!A && B) { //01
        return 4;
    } else {
        return 0;
    }
}

void yawIntHandler(void)
{
    //clear the interrupt
    GPIOIntClear(GPIO_PORTB_BASE, CHANNEL_A | CHANNEL_B);

    int32_t pinChange = GPIOIntStatus(GPIO_PORTB_BASE, true);

    //state machine to increment the counter
    ChA = GPIOPinRead(GPIO_PORTB_BASE, CHANNEL_A);
    ChB = GPIOPinRead(GPIO_PORTB_BASE, CHANNEL_B);

    //State Updating
    currentQuadState = getState(ChA, ChB);
    previousQuadState = getState(ChA_Previous, ChB_Previous);
   
    //direction determination
    if (
            (currentQuadState == 1 && previousQuadState == 4) ||
            (currentQuadState == 2 && previousQuadState == 1) ||
            (currentQuadState == 3 && previousQuadState == 2) ||
            (currentQuadState == 4 && previousQuadState == 3)) {
        clockwise = false;
        yawCount--;
    } else if (
            (currentQuadState == 4 && previousQuadState == 1) ||
            (currentQuadState == 3 && previousQuadState == 4) ||
            (currentQuadState == 2 && previousQuadState == 3) ||
            (currentQuadState == 1 && previousQuadState == 2)) {
        clockwise = true;
        yawCount++;
    }


    //counter reset if it does full rotation
    if(yawCount <= 0 ) {
        yawCount = QUAD_NUMBER_OF_TEETH - 1;
    } else if (yawCount >= QUAD_NUMBER_OF_TEETH - 1) {
        yawCount = 0;
    }
     
    //counter increments for each high pulse
    ChA_Previous = ChA;
    ChB_Previous = ChB;
    previousQuadState = currentQuadState;

}

void yawRefIntHandler(void)
{
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
    yawCount = 0;
    yawRefFound = true;
}

void initYaw(void)
{
    // Enable the GPIOB Peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    //Wait for the GPIOB module to be ready
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB) && !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC) )
    {
    }

    // Register the port-level interrupt handler.
    // This handler is the first level interrupt handler for all the pin interrupts
    GPIOIntRegister(GPIO_PORTB_BASE, yawIntHandler);
    GPIOIntRegister(GPIO_PORTC_BASE, yawRefIntHandler);

    // Initialize GPIO_PIN_0 (CHANNEL_A) and GPIO_PIN_1 (CHANNEL_B) as inputs.
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,
                         CHANNEL_A | CHANNEL_B);

    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
        

    // Make GPIO_PIN_0 (CHANNEL_A) and GPIO_PIN_1 (CHANNEL_B) both rising and falling edge triggered.
    GPIOIntTypeSet(GPIO_PORTB_BASE, CHANNEL_A | CHANNEL_B, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    
    // Store the initial state of GPIO_PIN_0 (CHANNEL_A) and GPIO_PIN_1 (CHANNEL_B)
    ChA_Previous = GPIOPinRead(GPIO_PORTB_BASE, CHANNEL_A);
    ChB_Previous = GPIOPinRead(GPIO_PORTB_BASE, CHANNEL_B);

    // Enable the pin interrupts.
    GPIOIntEnable(GPIO_PORTB_BASE, CHANNEL_A | CHANNEL_B);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4);
    //GPIOIntDisable(GPIO_PORTC_BASE, GPIO_PIN_4);

    //Set the yaw interupt priority
    IntPrioritySet(INT_GPIOC_TM4C123, YAW_REF_PRIORITY);
    IntPrioritySet(INT_GPIOB_TM4C123, YAW_PRIORITY);

}

int32_t yawInDegrees(void)
{
    int32_t degrees = ((yawCount * TOTAL_DEGREES * 10) / QUAD_NUMBER_OF_TEETH);
    //return the yaw as a negative number depending if more or less than 180deg
    if(yawCount < QUAD_NUMBER_OF_TEETH / 2) {
        return degrees;
    } else{
        return (degrees - TOTAL_DEGREES * 10);
    }
}


//getter and reset files for other modules. =============

int16_t getYawCount(void)
{
    return yawCount;
}

void resetYawRefFlag(void)
{
    yawRefFound = false;
}

bool getYawRefFlag(void)
{
    return yawRefFound;
}

