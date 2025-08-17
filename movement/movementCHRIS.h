/*
 * movement.h
 *
 *  Created on: Sep 12, 2024
 *      Author: itschris
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

#include "Timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "uart.h"
#include <stdio.h>
#include "grid.h"
#include <math.h>
#include "obstacle_detection.h"

// flag for if a bump/cliff sensor goes off
volatile int obstacle_hit;

// multiplier for wheel calibration
float wheel_calibrate_value;

void simpleHoleAvoid(position *current, int angle_turn);

void simpleObstacleAvoid(position *current);

void simpleEdgeAvoid(position *current);

int invisReact(position *bot);

int checkInvis();

// Moves the robot forward based on a given amount of mm.
int move_forward(position* current, int distance, char avoidence);

//void move_loop(int distance);

void move_forward_live_update(int distance, position* current);

void move_forward_live_update_no_avoidance(int distance, position *current);

void turn_clockwise_live_update(int degrees, position *current);

void turn_counterclockwise_live_update(int degrees, position *current);

void move_backward_live_update(int distance, position* current);

void move_backward(position* current, int distance);

//void turn_clockwise(oi_t* sensor, int degrees);
//
//void turn_counterclockwise(oi_t* sensor, int degrees);

void connected_movement(int dir);

int centistomillis(int centis);

void update_bot_pos(position *current, int distance);

#endif /* MOVEMENT_H_ */
