/*
 * grid.c
 *
 *  Created on: Nov 14, 2024
 *      Author: armondor
 *
 *      This file contains the grid data for the bot including items and position and locations.
 */

#define MAX_ITEMS 10 // max number of initalized items

#include "grid.h"

item_t items[MAX_ITEMS]; // array of items in the field
int num_items = 0; // number of items initalized

// initalizes the locations of charging station and delivery zone
void grid_init()
{
    charging_station.x = 180;  // center of bot at charging station
    charging_station.y = 180;

    delivery_zone.x = GRID_X / 2;
    delivery_zone.y = GRID_Y - 300;

    NULL_item.item = "NULL";
    NULL_item.coord.x = 0;
    NULL_item.coord.y = 0;
}


