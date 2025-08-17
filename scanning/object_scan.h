/*
 * object_scan.h
 *
 *  Created on: Dec 7, 2024
 *      Author: dawud
 */

#ifndef OBJECT_SCAN_H_
#define OBJECT_SCAN_H_

#include <stdio.h>
#include "sensor-data.h"
#include <string.h>
#include <math.h>
#include "uart.h"
#include "Timer.h"
#include "movementCHRIS.h"
#include "open_interface.h"
#include "adc.h"
#include "servo.h"
#include "obstacle_detection.h"

// struct that contains values for each object
typedef struct
{
    int start_angle;
    int end_angle;
    float start_dist;
    float end_dist;

} object_t;

// array of detected objects
object_t objects[50];
int num_objects;

// Performs a 180 degree scan, locate objects, and plot object edge coords
void scan_for_objects();

// scans 180 degrees using IR. Scans ping if IR < MAX_DIST.
void preform_scan();

// scan at delivery zone and calibrate bot pos
int delivery_zone_scan(position *bot);

#endif /* OBJECT_SCAN_H_ */
