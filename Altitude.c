/*************************************************************************************
Altitude.c - This module is used to initialise the altitude and define the range that 
             the helicopter will be able to move in. It averages the ADC values collected 
             within a circular buffer and returns the altitude of the helicopter as a 
             percentage value. 
             
Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 14th of May, 2024.
***************************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"

#include "Altitude.h"
#include "circBufT.h"

static int16_t groundVal = 0;   // Initialis the ground value from the ADC val
static int16_t topVal = 0;      // Initialise the top value based on the ground value

static uint32_t g_ulSampCnt;	// Counter for the interrupts
static circBuf_t g_inBuffer;    // Buffer of size BUF_SIZE integers (sample values)
static int32_t val = 0;         // Intermediate variable to update the ADC value
static int16_t ADCPercent;      // Initialised value to store the ADC value converted into a percentage


// Author of Function: P.J. Bones (UCECE)
int32_t updateADCVal(void) {
    int32_t sum = 0;
    int i;
    
    // Add the sum of all ADC values within the 20 length ciruclar buffer
    for (i = 0; i < BUF_SIZE; i++)
        sum = sum + readCircBuf(&g_inBuffer);
    
    // Calculate and display the rounded mean of the buffer contents
    return ((2 * sum + BUF_SIZE) / 2 / BUF_SIZE);
}

// Author of Function: P.J. Bones (UCECE)
void SysTickADCTrigger(void)
{
    // Initiate an interrupt in another module
    ADCProcessorTrigger(ADC0_BASE, 3); 
    g_ulSampCnt++;
}

// Author of Function: P.J. Bones (UCECE)
void ADCIntHandler(void)
{
	uint32_t ulValue;
	// Get the single sample from ADC0
	ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
	
	// Place it in the circular buffer (advancing write index)
	writeCircBuf (&g_inBuffer, ulValue);
	
	// Clean up, clearing the interrupt
	ADCIntClear(ADC0_BASE, 3);                          
}

// Author of Function: P.J. Bones (UCECE)
void initADC (void)
{
    /* 
    The ADC0 peripheral must be enabled for configuration and use.
    */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    /* 
    Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    will do a single sample when the processor sends a signal to start the
    conversion. 
    */
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  
    /*  
    - Configure step 0 on sequence 3.  
    - Sample channel 0 (ADC_CTL_CH0) in single-ended mode (default) and configure the 
        interrupt flag (ADC_CTL_IE) to be set when the sample is done. 
    - Tell the ADC logic that this is the last conversion on sequence 3 (ADC_CTL_END).  
    - Sequence 3 has only one programmable step.  
    - Sequence 1 and 2 have 4 steps, and sequence 0 has 8 programmable steps.  
    - Since we are only doing a single conversion using sequence 3 we will only configure step 0. 
    */
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                             ADC_CTL_END);    
                             
    /*
    Since sample sequence 3 is now configured, it must be enabled.
    */
    ADCSequenceEnable(ADC0_BASE, 3);
  
    /* 
    Register the interrupt handler
    */
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);
    IntPrioritySet(INT_ADC0SS3_TM4C123, ADC_PRIORITY);

    /* 
    Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    */
    ADCIntEnable(ADC0_BASE, 3);
	initCircBuf (&g_inBuffer, BUF_SIZE);
}

void setGroundTopVal (void) {
    val = updateADCVal();
    groundVal = val; // Make the ground value, the first value stored in Val when the ADC is initialised
    topVal = groundVal - ONE_VOLT_BITS_VAL; // Make the top value the maximum difference that can occur (1V equivalent)
}

void initalAltitudeSetup(void) 
{
    SysCtlDelay (1000000); // System delay to wait for the buffer to fill up with ADC sample values
    val = updateADCVal(); // Updates the value stored during intialisation

    setGroundTopVal(); 
}

int32_t getAverageADCVal(void)
{
    val = updateADCVal();
    return val;
}

int16_t getGround(void)
{
    return groundVal;
}

int16_t getTop(void)
{
    return topVal;
}

int16_t getADCPercentage(void)
{
    // Formula to claculate the ADC percentage 
    ADCPercent = ((groundVal - val ) * 100) / ONE_VOLT_BITS_VAL;
    return ADCPercent;
}
