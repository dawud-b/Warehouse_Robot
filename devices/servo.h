/*
 * servo.h
 *
 *  Created on: Nov 7, 2024
 *      Author: armondor
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <inc/tm4c123gh6pm.h>
#include "Timer.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "lcd.h"
#include "adc.h"
#include "button.h"
#include <Math.h>

// values set in main BEFORE initializing servo
int right_calibration_value;
int left_calibration_value;
// move servo to 0 after setting these values!!!

void servo_init();

void servo_move(float degrees);

void calibrate_servo();

int find_angle_value();



#endif /* SERVO_H_ */
