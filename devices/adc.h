/*
 * adc.h
 *
 *  Created on: Oct 24, 2024
 *      Author: armondor
 */

#ifndef ADC_H_
#define ADC_H_

#include <inc/tm4c123gh6pm.h>
#include "Timer.h"
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include "lcd.h"
#include "servo.h"
#include "button.h"
#include "uart.h"

void adc_init();

int adc_IR_read();

float find_IR_distance (int raw_ir);

void ping_init();

float ping_read();

void TIMER3B_Handler();

void calibrate_IR();

#endif /* ADC_H_ */
