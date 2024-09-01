#ifndef DISPLAY_H
#define DISPLAY_H

// The maximum range the helicopter can go
#define ONE_VOLT_BITS_VAL 1241

/*
Initalises the Orbit OLED display 
*/
void initDisplay(void);

/*
Function used to display the flight info 
*/
void displayFlightInfo(void);

#endif
