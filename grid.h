/*
 * grid.h
 *
 *  Created on: Nov 14, 2024
 *      Author: armondor
 *
 *
 */

#ifndef GRID_H_
#define GRID_H_

#include <stdio.h>
#include "Timer.h"
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "driverlib/interrupt.h"
#include "uart.h"
#include <string.h>

#define GRID_X 4256 // in mm
#define GRID_Y 2432 // in mm

//holds the current x and y value of the bot in millimeters, and the current orientaion of the bot. None of this should EVER go negative. If it does, then something has gone very wrong.
typedef struct{
    int x;
    int y;
    int deg;
    int quadrant;
    int prev_deg;
} position;

//simple coordinate.
typedef struct
{
    int x; // x value in mm
    int y; // y value in mm
} coord_t;


// current location of the bot
position bot_location;

// bot location when it hit an object, gets populated in invisReact before the bot moves
coord_t nono_zone;

// coord for delivery zone middle
coord_t delivery_zone;

// coord for charging station middle
coord_t charging_station;

// returns this if an invalid item is called
item_t NULL_item;

// initializes delivery zone and charging station locations
void grid_init();


#endif /* GRID_H_ */
