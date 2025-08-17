/*
*   navigation.h
*   authored by chris
*   #swagup
*   created 11/18/24
*/

#ifndef NAVIGATION_H_
#define NAVIGATION_H_

#include <stdio.h>
#include <math.h>
#include "movementCHRIS.h"
#include "grid.h"
#include "open_interface.h"
#include "uart.h"

void move_easiest(position* current, coord_t destination, oi_t* sensor);

void rotate_towards(position* current, coord_t destination);

void move_adapting(position* current, coord_t destination);

int calc_cw_degree_differential(int start, int end); 

#endif
