/*
*
*   uart.h
*
*   Used to set up the UART
*   uses UART1 at 115200
*
*
*   @author Dane Larson
*   @date 07/18/2016
*   Phillip Jones updated 9/2019, removed WiFi.h
*/

#ifndef UART_H_
#define UART_H_

#include "Timer.h"
#include <inc/tm4c123gh6pm.h>
#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "driverlib/interrupt.h"

volatile char uart_data; // data received through UART
volatile char flag; // is 1 if an interrupt has occurred

void uart_init(void);

void uart_sendChar(char data);

char uart_receive(void);

char uart_receive_nonblocking();

void uart_sendStr(const char *data);

void uart_interrupt_init();

void uart_interrupt_handler();


#endif /* UART_H_ */
