#ifndef PWM_H
#define PWM_H

#include <stdint.h>

#define PWM_PRESCALER   200
#define PWM_FREQUENCY   100
#define PWM_200HZ_CLOCK_COUNT 100000 

//  Main Rotor PWM: PC5
#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM  SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5

//PWM details Tail rotor: PF1
#define PWM_TAIL_BASE        PWM1_BASE
#define PWM_TAIL_GEN         PWM_GEN_2
#define PWM_TAIL_OUTNUM      PWM_OUT_5
#define PWM_TAIL_OUTBIT      PWM_OUT_5_BIT
#define PWM_TAIL_PERIPH_PWM  SYSCTL_PERIPH_PWM1
#define PWM_TAIL_PERIPH_GPIO SYSCTL_PERIPH_GPIOF
#define PWM_TAIL_GPIO_BASE   GPIO_PORTF_BASE
#define PWM_TAIL_GPIO_CONFIG GPIO_PF1_M1PWM5
#define PWM_TAIL_GPIO_PIN    GPIO_PIN_1

/*
    intialise PWM module to generate PWM signals
*/
void initPWM(void);

/*
Set the PWM duty cycle for the tail rotor
*/
void setTailPWM( uint8_t );

/*
Set the PWM duty cycle for the main rotor
*/
void setMainPWM( uint8_t);

/*
getter main rotor duty for other modules
*/
uint8_t getMainDuty(void);

/*
getter tail rotor duty for other modules
*/
uint8_t getTailDuty(void);

#endif //PWM.h
