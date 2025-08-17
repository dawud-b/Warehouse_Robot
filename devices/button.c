/*
 * button.c
 *
 *  Created on: Jul 18, 2016
 *      Author: Eric Middleton, Zhao Zhang, Chad Nelson, & Zachary Glanz.
 *
 *  @edit: Lindsey Sleeth and Sam Stifter on 02/04/2019
 *  @edit: Phillip Jone 05/30/2019: Mearged Spring 2019 version with Fall 2018
 */

//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"

// Global varibles
volatile int button_event;
volatile int button_num;

/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init()
{
    static uint8_t initialized = 0;

    //Check if already initialized
    if (initialized)
    {
        return;
    }

    // delete warning after implementing

    // Reading: To initialize and configure GPIO PORTE, visit pg. 656 in the
    // Tiva datasheet.

    // Follow steps in 10.3 for initialization and configuration. Some steps
    // have been outlined below.

    // Ignore all other steps in initialization and configuration that are not
    // listed below. You will learn more about additional steps in a later lab.

    // enable PORT E clock
    SYSCTL_RCGCGPIO_R |= 0b00010000;

    // set wires 0:3 as input by clearing bits to 0. Preserve other bits.
    GPIO_PORTE_DIR_R &= 0xF0;

    // enable digital function for bits 0:3. Preserve other bits.
    GPIO_PORTE_DEN_R |= 0x0F;

    initialized = 1;
}

/**
 * Initialize and configure PORTE interupts
 */
void init_button_interrupts()
{


    // In order to configure GPIO ports to detect interrupts, you will need to visit pg. 656 in the Tiva datasheet.
    // Notice that you already followed some steps in 10.3 for initialization and configuration of the GPIO ports in the function button_init().
    // Additional steps for setting up the GPIO port to detect interrupts have been outlined below.
    // TODO: Complete code below

    // 1) Mask the bits for pins 0-3
    GPIO_PORTE_IM_R &= 0xF0;

    // 2) Set pins 0-3 to use edge sensing
    GPIO_PORTE_IS_R &= 0xF0;

    // 3) Set pins 0-3 to use both edges. We want to update the LCD
    //    when a button is pressed, and when the button is released.
   GPIO_PORTE_IBE_R |= 0x0F;

    // 4) Clear the interrupts
    GPIO_PORTE_ICR_R = 0x0F;

    // 5) Unmask the bits for pins 0-3
    GPIO_PORTE_IM_R |= 0x0F;


    // TODO: Complete code below
    // 6) Enable GPIO port E interrupt
    NVIC_EN0_R |= 0x00000010;

    // Bind the interrupt to the handler.
    IntRegister(INT_GPIOE, gpioe_handler);
}

/**
 * Interrupt handler -- executes when a GPIO PortE hardware event occurs (i.e., for this lab a button is pressed)
 */
void gpioe_handler()
{
    // Clear interrupt status register
    GPIO_PORTE_ICR_R = 0x0F;
    // update button_event = 1;
    button_num = button_getButton();
}

/**
 * Returns the position of the rightmost button being pushed.
 * @return the position of the rightmost button being pushed. 4 is the rightmost button, 1 is the leftmost button.  0 indicates no button being pressed
 */
uint8_t button_getButton()
{
    //  0x01, 0x02, 0x04, 0x08

    // if button 4 is pressed
    if (~GPIO_PORTE_DATA_R & 0x08)
    {
        return 4;
    }
    // if button 3 is pressed
    else if (~GPIO_PORTE_DATA_R & 0x04)
    {
        return 3;
    }
    // if button 2 is pressed
    else if (~GPIO_PORTE_DATA_R & 0x02)
    {
        return 2;
    }
    // if button 1 is pressed
    else if (~GPIO_PORTE_DATA_R & 0x01)
    {
        return 1;
    }
    // no button pressed
    else
    {
        return 0;
    }
}

