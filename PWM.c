/*****************************************************************************
PWM.c - This modules controls the PWM generation periperals.
        it will generate the PWM signal at the required duty cycle 
        based of the duty cycle passed to it. 

Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 14th of May, 2024.
*********************************************************************************/



#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "buttons4.h"

#include "pid.h"
#include "PWM.h" 

uint8_t tailDuty = 0 ;
uint8_t mainDuty = 0;

void setTailPWM( uint8_t duty )
{
    //Store the duty cycle as global
    tailDuty = duty;
    //Calculate PWM frequency
    uint32_t tailPWMPeriod = SysCtlClockGet() / PWM_PRESCALER;
    //Set the period and duty cycle
    PWMGenPeriodSet(PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_200HZ_CLOCK_COUNT);
    PWMPulseWidthSet(PWM_TAIL_BASE, PWM_TAIL_OUTNUM, tailPWMPeriod * duty / 100);
}

void setMainPWM( uint8_t duty )
{
    //store the main duty cycle
    mainDuty = duty;
    //calculate the PWM frequency
    uint32_t  mainPWMPeriod = SysCtlClockGet() / PWM_PRESCALER;
    //Set the period and duty cycle
    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, mainPWMPeriod);
    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM, mainPWMPeriod * duty / 100);
}

void initPWM(void)
{
    //Tail on PF1
    //Main on PC5
    //Enable PWM module and GPIO pad enable
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);
    SysCtlPeripheralEnable(PWM_TAIL_PERIPH_GPIO);

    //configure GPIO pins
    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinConfigure(PWM_TAIL_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);
    GPIOPinTypePWM(PWM_TAIL_GPIO_BASE, PWM_TAIL_GPIO_PIN);

    //PWM generation configure
    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenConfigure(PWM_TAIL_BASE, PWM_TAIL_GEN, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);
    PWMGenEnable(PWM_TAIL_BASE, PWM_TAIL_GEN);

    //Turn on PWM generation
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);
    PWMOutputState(PWM_TAIL_BASE, PWM_TAIL_OUTBIT, true);
}


//getter functions for other modules ==================== 

uint8_t getMainDuty(void)
{
    return mainDuty;
}

uint8_t getTailDuty(void)
{
    return tailDuty;
}

