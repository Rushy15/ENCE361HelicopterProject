#ifndef KERNEL_H
#define KERNEL_H

// Defining UART port setup 
#define BAUD_RATE 9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX
#define MAX_STR_LEN 300 

// Defining timer port setup
#define TIMER_CONFIG TIMER_CFG_PERIODIC
#define TIMER_INT_TYPE TIMER_TIMA_TIMEOUT

#define UART_BASE TIMER0_BASE
#define BUTTON_BASE TIMER1_BASE
#define DISPLAY_BASE TIMER2_BASE
#define PID_BASE TIMER3_BASE

#define MILLISECONDS 1000
#define UART_PERIOD_MS 250
#define BUTTON_PERIOD 10
#define DISPLAY_PERIOD 100
#define PID_PERIOD 15

//Some thought to go into this
#define UART_INT_PRIORITY 2
#define BUTTON_INT_PRIORITY 1
#define DISPLAY_PRIORITY 3
#define PID_PRIORITY 1

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

#define SWITCH1_PERIPH          SYSCTL_PERIPH_GPIOA
#define SWITCH_BASE             GPIO_PORTA_BASE
#define SWITCH1_PIN             GPIO_PIN_7
#define SWITCH2_PIN             GPIO_PIN_6



/*
Enum for the flight mode
*/
typedef enum {
    LANDED = 0,
    TAKE_OFF,
    FLYING,
    LANDING
} Flight_mode_t;


/*
Interrupt handler for displaying flight information through serial port
*/
void uartInteruptTask(void);

/*
Interrupt handler polling the buttons
*/
void buttonPollInteruptTask(void);

/*
Interrupt handler for displaying information on the OLED Screen on the TIVA Board 
*/
void oledDisplayInteruptTask(void);

/*
Interrupt handler for PID timer
*/
void pidUpdateInteruptTask(void);

/*
Initalise the kernel and all clock modules
*/
void initKernel(void);

/*
Initialises all the switches used on the TIVA Board
*/
void initSwitch(void);

/*
Gets the state of SW1
*/
bool getSwitchState(void);

/*
Sets the flight mode for the FSM
*/
void setFlightMode(Flight_mode_t);

#endif
