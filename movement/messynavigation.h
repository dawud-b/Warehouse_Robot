/*
* messynavigation.h
* created by chris - 11/29/24
*/

#ifndef MESSYNAVIGATION_H_
#define MESSYNAVIGATION_H_

#include "movementCHRIS.h"
#include "sensor-data.h"
#include "uart.h"
#include "adc.h"
#include "servo.h"
#include "obstacle_detection.h"
#include "object_scan.h"
#include "navigation.h"
#include <math.h>

//saves a gap as the midpoint and the gap size between the two.
typedef struct{
    int gap;
    coord_t midpt;
} gap_point_t;

//array of gaps and the number of gaps found.
gap_point_t gaps[50];
int gap_number;


// call to reset gap_number
void messy_nav_init();

void nav_to_by_max_thirty_edge_coords(position *current, coord_t destination, int dist);

int hit_move(position *current, coord_t destination);

int min_i(int a, int b);

float min_f(float a, float b);

int max_i(int a, int b);

float max_f(float a, float b);

#endif
