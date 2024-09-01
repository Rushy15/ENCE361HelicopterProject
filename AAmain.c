/*************************************************************************************
AAmain.c - This is the main module used control the FSM, initalise all the modules that
            are needed to run the program, and sets up the 20MHz clock used for the 
            program.

Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 14th of May, 2024.
***************************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "driverlib/timer.h"
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
#include "Display.h"
#include "buttons4.h"
#include "Yaw.h"
#include "pid.h"
#include "PWM.h"
#include "kernel.h"
#include "setpoint.h"



//*****************************************************************************
// Constants
//*****************************************************************************

#define SAMPLE_RATE_HZ 120
#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4

#define SYSTICK_PRIORITY 0

/* 
ADC Trigger Interrupt Handler
*/
void SysTickIntHandler(void)
{
    SysTickADCTrigger();
}

/*
Initialising the 20MHz Clock and Systick Interrupt
*/
void initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);

    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);

    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //IntPrioritySet(FAULT_SYSTICK, SYSTICK_PRIORITY);

    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}


int main(void)
{
    // initialising modules to setup system
    initClock();
    initADC();
    initYaw();
    initDisplay ();
    initPWM();
    initButtons ();
    initalAltitudeSetup();
    initPID();
    initSetpoint();
    initKernel();
    initSwitch();


    // Enable interrupts to the processor.
    IntMasterEnable();
    Flight_mode_t flightMode = LANDED;

    // Flags for intital setup and changing states
    bool hovering = false;
    bool initial = true;

    //state machine to manage the flight mode
    while(1)
    {
        // Get the system information 
        bool switchState = getSwitchState(); 
        int16_t altSet = getAltitudeSetpoint();
        int32_t altitude = getAverageADCVal();
        int32_t groundVal = getGround();

        switch (flightMode) {
            case LANDED:
                // Reset flags
                hovering = false;
                resetYawRefFlag();
                
                // Turn the motors off
                setMainPWM(0);
                setTailPWM(0);

                // Conditions to switch states
                if (switchState) {
                    flightMode = TAKE_OFF;
                } else {
                    flightMode = LANDED;
                }
                
                // State Indication LEDs
                GPIOPinWrite(GPIO_PORTF_BASE,  BLUE_LED, BLUE_LED);
                GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, 0x00);
                break;


            case TAKE_OFF:
                // Set to hovering altitude ~10%
                GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, GREEN_LED);
                GPIOPinWrite(GPIO_PORTF_BASE,  BLUE_LED, 0x00);

                setAltitude(groundVal - (ONE_VOLT_BITS_VAL / 100));

                // Wait till altitude is reached
                if (!hovering) {
                    if (altSet == altitude) { // getMainError() < 5
                        hovering = true;
                    }
                }
                if (initial) {
                    setTailPWM(20);
                }

                if (hovering && initial) {
                    //TimerIntEnable(STATE_MACHINE_BASE, TIMER_INT_TYPE);
                    initial = false;
                    }
                
                // Conditions to switch states
                if (getYawRefFlag() && hovering) {
                    setYaw(0);
                    //TimerIntDisable(STATE_MACHINE_BASE, TIMER_INT_TYPE);
                    flightMode = FLYING;
                } else if (!switchState){
                    flightMode = LANDING;
                } else {
                    flightMode = TAKE_OFF;
                }
                break;


        case FLYING:
            GPIOPinWrite(GPIO_PORTF_BASE,  BLUE_LED, BLUE_LED);
            GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, GREEN_LED);
            resetYawRefFlag();
            // Need to enable inputs to change altitude and yaw.
            if (!switchState) {
                flightMode = LANDING;
            } else {
                flightMode = FLYING;
            }
            break;

        case LANDING:
            // Re-initialise yaw reference bool to find the reference point again
            GPIOPinWrite(GPIO_PORTF_BASE,  BLUE_LED, 0x00);
            GPIOPinWrite(GPIO_PORTF_BASE,  GREEN_LED, 0x00);

            setYaw(0);
            // Wait till the yaw setpoint is at 0.
            if (getYawRefFlag() || (getYawCount() == 0)) {
                setAltitude(groundVal);
            }

            // Conditions to switch states.
            if ((altitude == groundVal) && getYawRefFlag()) {
                flightMode = LANDED;
            } else {
                flightMode = LANDING;
            }
            break;
        }
        setFlightMode(flightMode);
    }
}
