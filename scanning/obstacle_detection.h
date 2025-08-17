/*
 * obstacle_detection.h
 *
 *  Created on: Dec 2, 2024
 *      Author: dawud
 */

#ifndef OBSTACLE_DETECTION_H_
#define OBSTACLE_DETECTION_H_

#include "navigation.h"
#include "grid.h"
#include <stdio.h>
#include "sensor-data.h"
#include <string.h>
#include <math.h>
#include "uart.h"
#include "Timer.h"
#include "open_interface.h"
#include "adc.h"
#include "servo.h"

#define CENTER_TO_SERVO 105.0 // distance from center of bot to middle of scanner in mm

// array of obstacle points
coord_t obstacles_arr[100];

// number of obstacles stored in the array. Last obstacle found: obstacles_arr[num_obstacles - 1]
int num_obstacles;

// finds the coord of obstacle point scanned at servo_angle and ping_dist.
// stores this coord in obstacles_arr
void find_obstacle_location(position *bot_pos, int ping_dist, int servo_angle);

// servo grid position calculated from bots midpoint
coord_t find_servo_pos(position *bot_pos);

// finds the distance from the center of the bot to the obstacle point scanned at ping_dist and servo_angle
// returns distance in mm
int find_bot_obstacle_dist(int ping_dist, int servo_bot_angle);

// finds the angle between the bot and the obstacle point passed
// basically, how much the bot needs to move to point towards the obstacle
// positive for clockwise, negative for counterClockwise
// returns angle in degrees
int find_angle_to_obstacle(position *bot_pos, coord_t obstacle_pos,
                           int ping_dist, int servo_bot_angle);

// same as above but finds it based on coord_t instead of servo angle and dist.
int find_angle_to_obstacle_coord(position *bot_pos, coord_t obstacle_pos);

//used to calibrate the cliff sensor, shows the user the values of each cliff sensor
void cliff_calibrate();

float find_resultant(coord_t a, coord_t b);

void create_nono(position* bot, int invis);

#endif /* OBSTACLE_DETECTION_H_ */
