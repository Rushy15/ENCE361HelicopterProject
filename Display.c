/*************************************************************************************
Display.c - This module is used to write to the OLED display on the TIVA Board.
             
Authors: 
-> Aarush Bhagwat (54894013) 
-> Toby Dean (72894216).

Last modified: 14th of May, 2024.
***************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils/ustdlib.h"
#include "driverlib/debug.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "OrbitOLED/lib_OrbitOled/OrbitOLED.h"
#include "Display.h"
#include "Altitude.h"
#include "Yaw.h"
#include "setpoint.h"
#include "pid.h"
#include "PWM.h"

void initDisplay (void)
{
    OLEDInitialise (); // Initialise the display
}

void displayFlightInfo(void)
{
    int32_t percentVal = getADCPercentage();    // Get the altitude value as a percentage
    int32_t count = yawInDegrees();             // Get the yaw value in degrees
    
    char string[17];  // 16 characters across the display

    // Prints the altitude as a percentage (Line 1)
    usnprintf (string, sizeof(string), "Alt = %4d%%", percentVal);
    OLEDStringDraw (string, 0, 0);

    // Prints the Yaw in degrees with sub-degree precision (Line 2)
    usnprintf (string, sizeof(string), "Yaw = %2d.%1d Deg", count/10, abs(count % 10));
    OLEDStringDraw (string, 0, 1);

    // Prints the Duty Cycle of the main rotor as a percentage (Line 3)
    usnprintf (string, sizeof(string), "Duty Main = %2d%%", getMainDuty());
    OLEDStringDraw (string, 0, 2);

    // Prints the Duty Cycle of the tail rotor as a percentage (Line 4)
    usnprintf (string, sizeof(string), "Duty Tail = %2d%%", getTailDuty());
    OLEDStringDraw (string, 0, 3);
}
