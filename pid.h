#ifndef PID_H
#define PID_H

#include <stdlib.h>

//Define coupling constants and divisors 
#define GRAVITY_CONSTANT 40
#define COUPLING_CONSTANT 8
#define COUPLING_DIVISOR 10
#define DIVISOR 1000000

//define the max range for the main rotor
#define MIN_MAIN_DUTY 30
#define MAX_MAIN_DUTY 90

//define the max range for the tail rotor
#define MIN_TAIL_DUTY 15
#define MAX_TAIL_DUTY 50

//define the error range for yaw intergral term
#define ERROR_RANGE 20


/*
This is a gain clamp for the main rotor to ensure that there is no integral windup.
*/
void mainGainClamp(void);

/*
Performs the calculation to calculate the output control that should be passed to the
main rotor based on gains tuned for the main rotor using PID control.
*/
int8_t mainRotorController(void);

/*
Performs the calculation to calculate the output control that should be passed to the
tail rotor based on gains tuned for the tail rotor using PID control.
*/
int8_t tailRotorController(void);


/*
initalise the pid module.
Will set the previous values for intergral and derivative controller to avoid errors
*/
void initPID(void);

#endif
