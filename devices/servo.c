/*
 * servo.c
 *
 *  Created on: Nov 7, 2024
 *      Author: armondor
 */

#include "servo.h"

//finds number of cycles for input angle degree
int find_MATCH_value(float degrees)
{
    return (int) ((left_calibration_value - right_calibration_value)
            * (degrees / 180)) + right_calibration_value;
}

// returns the angle of the servo
int find_angle_value()
{
    int current_match = TIMER1_TBMATCHR_R & 0x00FFFF;
    current_match += (TIMER1_TBPMR_R & 0x00FF) << 16;

    return ((current_match - right_calibration_value)
            / (left_calibration_value - right_calibration_value)) * 180;
}

// initiates the servo for use
void servo_init()
{
    SYSCTL_RCGCGPIO_R |= 0b00000010; //enable port B clock
    SYSCTL_RCGCTIMER_R |= 0x02; //enable clock for timer 1

    GPIO_PORTB_DEN_R |= 0x20; //enable PB5
    GPIO_PORTB_DIR_R |= 0x20; //enable PB5 for output
    GPIO_PORTB_AFSEL_R |= 0x20; // alt func for PB5
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFF0FFFFF) | 0x00700000; //set PB5 to T1CCP1 function

    TIMER1_CTL_R &= ~0x4100; // disable TIMER1B, unaffected PWM dir
    TIMER1_CFG_R = 0x04; // 16 bit timer
    TIMER1_TBMR_R = 0x80A; // Periodic Mode, PWM mode
    TIMER1_TBPR_R |= 0x05; //set value for prescaler
    TIMER1_TBILR_R = 0x5F00; //set period for PWM mode, period of 22 ms has a count of 352,000 cycles
    int match_val = find_MATCH_value(0);
    TIMER1_TBMATCHR_R = (match_val & 0x00FFFF);
    TIMER1_TBPMR_R = (match_val & 0xFF0000) >> 16;
    timer_waitMillis(500);
    TIMER1_CTL_R |= 0x0100; // enable TIMER1B, unaffected PWM dir

}

// moves the servo the specified angle
void servo_move(float degrees)
{
    static int previous_degree = 0;
    float wait_time = abs(degrees - previous_degree) * 10;
    int match_val = find_MATCH_value(degrees);
    TIMER1_TBMATCHR_R = (match_val & 0x00FFFF);
    TIMER1_TBPMR_R = (match_val & 0xFF0000) >> 16;
    timer_waitMillis(wait_time);
    previous_degree = degrees;
}

// returns the value of the servos angle, for use in calibration
int calibrate_match_val()
{
    int current_match = TIMER1_TBMATCHR_R & 0x00FFFF;
    current_match += (TIMER1_TBPMR_R & 0x00FF) << 16;

    int button = button_getButton();

    while (button != 4)
    {
        button = button_getButton();

        if (button == 1) // decrease the servo match value, moves the servo left
        {
            current_match -= 10;
            lcd_printf("%d", current_match);
            TIMER1_TBMATCHR_R = (current_match & 0x00FFFF);
            TIMER1_TBPMR_R = (current_match & 0xFF0000) >> 16;
        }
        if (button == 2) // increase the servo match value, moves the servo right
        {
            current_match += 10;
            lcd_printf("%d", current_match);
            TIMER1_TBMATCHR_R = (current_match & 0x00FFFF);
            TIMER1_TBPMR_R = (current_match & 0xFF0000) >> 16;
        }
    }

    return current_match;
}

// calibration for the servo
void calibrate_servo()
{
    button_init();
    lcd_init();
    lcd_printf("SW1: Move Left\nSW2: Move Right\nSW4: Done\n Find 0 degrees");

    // Initialize servo - sets to zero
    servo_init();

    int right_val;
    right_val = calibrate_match_val();

    lcd_printf("SW1: Move Left\nSW2: Move Right\nSW4: Done\n Find 180 degrees");

    timer_waitMillis(100);
    int left_val;
    left_val = calibrate_match_val();

    lcd_printf("Save these values:\nRight: %d\nLeft: %d", right_val, left_val);

    timer_waitMillis(10000);
}
