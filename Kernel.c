/*************************************************************************************
Kernel.c - This module acts as the operating system for the microcontroller. It is used 
           to interact with peripherals and initialises all a range of timers to run all 
           the helicopter tasks at specific frequencies.

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
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/uart.h"
#include "driverlib/pin_map.h"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "OrbitOLED/lib_OrbitOled/OrbitOLED.h"
#include "inc/tm4c123gh6pm.h" 

#include "Altitude.h"
#include "Display.h"
#include "buttons4.h"
#include "Yaw.h"
#include "PWM.h"
#include "setpoint.h"
#include "pid.h"
#include "kernel.h"

Flight_mode_t flightMode;
bool switchState = false;
char statusStr[MAX_STR_LEN + 1];


void initSwitch(void)
{
    //set up the switch for state control
    SysCtlPeripheralEnable (SWITCH_BASE);
    GPIOPinTypeGPIOInput (SWITCH_BASE, SWITCH1_PIN);
    GPIOPadConfigSet (SWITCH_BASE, SWITCH1_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);

    //Sw2 for debugging
    SysCtlPeripheralEnable (SWITCH_BASE);
    GPIOPinTypeGPIOInput (SWITCH_BASE, SWITCH2_PIN);
    GPIOPadConfigSet (SWITCH_BASE, SWITCH2_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
}

void initialiseUSB_UART (void)
{

    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(UART_USB_PERIPH_UART);
    SysCtlPeripheralEnable(UART_USB_PERIPH_GPIO);

    while (!(SysCtlPeripheralReady(UART_USB_PERIPH_UART) && SysCtlPeripheralReady(UART_USB_PERIPH_GPIO))) {}


    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(UART_USB_GPIO_BASE, UART_USB_GPIO_PINS);
    GPIOPinConfigure (GPIO_PA0_U0RX);
    GPIOPinConfigure (GPIO_PA1_U0TX);

    UARTConfigSetExpClk(UART_USB_BASE, SysCtlClockGet(), BAUD_RATE,
            UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
            UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART_USB_BASE);
    UARTEnable(UART_USB_BASE);
}

void UARTSend (char *pucBuffer)
{
    // Loop while there are more characters to send.
    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART_USB_BASE, *pucBuffer);
        pucBuffer++;
    }
    

}


void uartInteruptTask(void)
{
    TimerIntClear(UART_BASE, TIMER_INT_TYPE);
    int16_t groundVal = getGround();
    int32_t count = yawInDegrees();

    // Stores enum flight states as a list of strings, and index's into list to get current flight mode. 
    const char *flghtState[] = {"Landed", "Take-off", "Flying", "Landing"};
    usprintf (statusStr, "Duty main: %d Duty tail %d | Yaw SP: %d Current Yaw : %2d.%1d (deg) | Alt SP: %d, Height per %d | flightmode: %s\r\n",
        getMainDuty(),
        getTailDuty(), 
        ((getYawSetpoint() * TOTAL_DEGREES) / QUAD_NUMBER_OF_TEETH),
        count/10, abs(count % 10),
        ((groundVal-getAltitudeSetpoint() ) * 100) / ONE_VOLT_BITS_VAL, 
        getADCPercentage(),
        flghtState[flightMode]);

    UARTSend (statusStr);
}

void buttonPollInteruptTask(void)
{
     TimerIntClear(BUTTON_BASE, TIMER_INT_TYPE);
     uint8_t butUpState;
     uint8_t butLeftState;
     uint8_t butRightState;
     uint8_t butDownState;


    switchState = GPIOPinRead(SWITCH_BASE, SWITCH1_PIN);

    

    if (GPIOPinRead(SWITCH_BASE,SWITCH2_PIN))
    {
        SysCtlReset();
    }

    if (switchState && !getYawRefFlag()) {
        updateButtons ();
        butUpState = checkButton (UP);
        butLeftState = checkButton (LEFT);
        butRightState = checkButton (RIGHT);
        butDownState = checkButton (DOWN);
        
        // update the altitude setpoint
        if (butLeftState == PUSHED) {
            incrementYawSetpoint();
        }
        if (butRightState == PUSHED) {
        decrementYawSetpoint();
        }
        
        // update yaw setpoint
        if (butUpState == PUSHED) {
        incrementAltitudeSetpoint();
        }

        if (butDownState == PUSHED) {
        decrementAltitudsetpoint();
        }
    }
}

void oledDisplayInteruptTask(void)
{
    //display info on oled
    TimerIntClear(DISPLAY_BASE, TIMER_INT_TYPE);
    displayFlightInfo();
}

//interupt handler for PID timer
void pidUpdateInteruptTask(void)
{
    //update the PWM output using PID controller
    TimerIntClear(PID_BASE, TIMER_INT_TYPE);
    setMainPWM(mainRotorController());
    // Only put effort on tail rotor is it is either flying or landing
    if(flightMode == FLYING || flightMode == LANDING) {
        setTailPWM(tailRotorController());
    }


}

void initKernel(void)
{
    initialiseUSB_UART();
    uint32_t clockFreq = SysCtlClockGet();

    //UArt timer 5hz low priority: timer 0, full width, periodic counter
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(UART_BASE, TIMER_CONFIG);
    uint32_t uartLoad = ((uint64_t)clockFreq * UART_PERIOD_MS) / MILLISECONDS;
    TimerLoadSet(UART_BASE,TIMER_A, uartLoad);
    TimerEnable(UART_BASE, TIMER_BOTH);

    //Register the interupts
    TimerIntRegister(UART_BASE, TIMER_A, uartInteruptTask);
    TimerIntEnable(UART_BASE, TIMER_INT_TYPE);

    //set interupt piority (from hw_ints)
    IntPrioritySet(INT_TIMER0A_TM4C123, UART_INT_PRIORITY);

    //button polling timer 50hz - mid priority
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(BUTTON_BASE, TIMER_CONFIG);
    uint32_t buttonLoad = (clockFreq * BUTTON_PERIOD) / MILLISECONDS;
    TimerLoadSet(BUTTON_BASE,TIMER_A, buttonLoad);
    TimerEnable(BUTTON_BASE, TIMER_BOTH);

    //Register the interupts
    TimerIntRegister(BUTTON_BASE, TIMER_A, buttonPollInteruptTask);
    TimerIntEnable(BUTTON_BASE, TIMER_INT_TYPE);

    //set interupt piority (from hw_ints)
    IntPrioritySet(INT_TIMER1A_TM4C123, BUTTON_INT_PRIORITY);

    //display polling timer 10hz - low priority
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    TimerConfigure(DISPLAY_BASE, TIMER_CONFIG);
    uint32_t displayLoad = (clockFreq * DISPLAY_PERIOD) / MILLISECONDS;
    TimerLoadSet(DISPLAY_BASE, TIMER_A, displayLoad);
    TimerEnable(DISPLAY_BASE, TIMER_BOTH);

    //Register the interupts
    TimerIntRegister(DISPLAY_BASE, TIMER_A, oledDisplayInteruptTask);
    TimerIntEnable(DISPLAY_BASE, TIMER_INT_TYPE);

    //set interupt piority (from hw_ints)
    IntPrioritySet(INT_TIMER2A_TM4C123, DISPLAY_PRIORITY);

    //PID control timer 100hz - high priority
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    TimerConfigure(PID_BASE, TIMER_CONFIG);
    uint32_t pidLoad = (clockFreq * PID_PERIOD) / MILLISECONDS;
    TimerLoadSet(PID_BASE, TIMER_A, pidLoad);
    TimerEnable(PID_BASE, TIMER_BOTH);

    //Register the interupts
    TimerIntRegister(PID_BASE, TIMER_A, pidUpdateInteruptTask);
    TimerIntEnable(PID_BASE, TIMER_INT_TYPE);

    //set interupt piority (from hw_ints)
    IntPrioritySet(INT_TIMER3A_TM4C123, PID_PRIORITY);


    /*
    LED initialisations for state identification on board
    */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPadConfigSet(GPIO_PORTF_BASE, GREEN_LED, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
	GPIODirModeSet(GPIO_PORTF_BASE, GREEN_LED, GPIO_DIR_MODE_OUT);
    GPIOPadConfigSet(GPIO_PORTF_BASE, BLUE_LED, GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
    GPIODirModeSet(GPIO_PORTF_BASE, BLUE_LED, GPIO_DIR_MODE_OUT);
}


bool getSwitchState(void)
{
    return switchState; // Gets the state of SW1
}


void setFlightMode(Flight_mode_t fmode)
{
    flightMode = fmode; // Gets the current flight mode from FSM 
}

