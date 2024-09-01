#ifndef ALTITUDE_H
#define ALITIUDE_H

#include <stdint.h>

#define ONE_VOLT_BITS_VAL 1241 // Total range of our ADC value
#define BUF_SIZE 20            // Buffer size to calculate the average ADC value
#define ADC_PRIORITY 0         // Priority level for the ADC interrupt

/*
Systick ADC Conversion Initialiser
*/
void SysTickADCTrigger(void);

/*
ADC Interrupt Handler
*/
void ADCIntHandler(void);

/*
Initialising the ADC Module
*/
void initADC(void);

/*
Sets the ground value
- This is equivalent to 0% Altitude.
*/
void setGroundTopVal(void);

/*
Updates the average ADC value by re-reading the buffer
*/
int32_t updateADCVal(void);


/*
Sets up the altitude by waiting for the buffer to fill up
*/
void initalAltitudeSetup(void);

/* 
Returns the average ADC value from the buffer
*/
int32_t getAverageADCVal(void);

/*
Returns the ground ADC value (i.e. the lower bounds)
(i.e. Altitude = 0%)
*/
int16_t getGround(void);

/*
Return the top value for the Helicopter upper bound 
(i.e. Altitude = 100%)
*/
int16_t getTop(void);

/*
Returns the ADC value as a percentage
*/
int16_t getADCPercentage(void);

#endif // ALTITUDE.H
