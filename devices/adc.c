/*
 * adc.c
 *
 *  Created on: Oct 24, 2024
 *      Author: armondor
 */
#include "adc.h"

#define CLOCK_SPEED 16000000

volatile unsigned int ping_sent;
volatile unsigned int ping_received;
volatile int adc_flag = 0;
volatile int edge = 1; // if 1 positive edge, if 0 negative edge

int overflow_count = 0;


// ------------------- ping and raw-IR values for calibration ----------------------------------------------
float ping_values[10] = { 9.08, 12.6, 17.43, 22.75, 27.97, 32.96, 39.11, 45.48, 49.03, 51.0 }; // bot 2041-03
int ir_values[10] = { 3238, 2498, 1849, 1443, 1276, 830, 644, 713, 652, 598 };

//float ping_values[10] = {12.36, 11.98, 19.40, 30.08, 37.86, 40.81, 43.70, 46.25, 51.08, 55.11}; // bot 2041-15
//int ir_values[10] = {2709, 2715, 1612, 1137, 911, 844, 812, 733, 692, 651};

//float ping_values[10] = {11.20, 16.63, 22.37, 26.80, 30.37, 35.76, 40.25, 44.57, 50.60, 55.23}; // bot 2041-14
//int ir_values[10] = {1477, 991, 729, 596, 487, 383, 321, 261, 206, 186};

//float ping_values[10] =  {11.32, 16.08, 21.57, 27.91, 30.91, 28.30, 42.63, 28.25, 27.88, 55.18}; // bot 2041-00
//int ir_values[10] =  {2971, 2135, 1600, 1203, 1045, 919, 785, 669, 612, 596};


// ---------------------------------------------------------------------------------------------------------

// IR Sensor initilization
void adc_init()
{
    SYSCTL_RCGCADC_R |= 0b0001; // enables ADC0 clock
    SYSCTL_RCGCGPIO_R |= 0b00000010; // enable GPIO port B clock

    while ((SYSCTL_PRGPIO_R & 0x02) != 0x02)
    {
    } //wait for GPIO clock to be enabled

    GPIO_PORTB_DIR_R &= 0b11101111; //enable PB4 as input
    GPIO_PORTB_AFSEL_R |= 0b00010000; // set alternate functions for PB4
    GPIO_PORTB_DEN_R &= 0b11101111; // disables digital functions for PB4
    GPIO_PORTB_AMSEL_R |= 0b00010000; // enable analog functions for PB4
    GPIO_PORTB_ADCCTL_R &= 0x00; //ADC is not triggered by pins, 0 by default

    while ((SYSCTL_PRADC_R & 0x0001) != 0x0001)
    {
    } //wait for ADC clock to be enabled

    ADC0_PC_R = (ADC0_PC_R & ~0xF) | 0x1; //
    ADC0_SSPRI_R = 0x0123; // SS3 highest priority,

    ADC0_ACTSS_R &= 0b0111; // disables ADC0 SS3 sample sequencers
    ADC0_EMUX_R &= 0x0FFF; // Sets SS3 trigger to PSSI initiate
    ADC0_SSMUX3_R = (ADC0_SSMUX3_R & 0x0) | 0xA; // sets a sequence of 1 AIN10 sample
    ADC0_SSCTL3_R = 0x6; // sets sample control bits: sample 1 is END and interrupt
    ADC0_IM_R &= 0x8; // enable SS3 interrupts
    ADC0_SAC_R = 0x6; // averager 64 samples

    // interrupt initialization
    ADC0_ISC_R = 0x8; // clear SS3 interrupts
    NVIC_EN0_R |= 1 << 17; // set bit 17 for enabling ADC0_SS3 interrupt

    ADC0_ACTSS_R |= 0b1000; // enable SS3
}

// Read IR scan
int adc_IR_read()
{
    int FIFO_val = 0;
    ADC0_PSSI_R |= 0b1000; // initiate SS0

    // wait till ADC conversions are finished
    while ((ADC0_RIS_R & 0x8) == 0)
    {
    }
    FIFO_val = ADC0_SSFIFO3_R & 0xFFF; // read conversion from FIFO
    ADC0_ISC_R |= 0x8;  // clear interrupt

    return FIFO_val; // return the adc value in the FIFO
}

// Function to convert ADC value to distance using arrays
float find_IR_distance(int raw_ir)
{
    int i;
    if (raw_ir >= ir_values[0])
    {
        return ping_values[0];
    }
    if (raw_ir <= ir_values[9])
    {
        return 60;
    }
    for (i = 0; i < 10; i++)  // Iterate through the array
    {
        if (raw_ir <= ir_values[i] && raw_ir > ir_values[i + 1])
        {
            float slope = (ping_values[i + 1] - ping_values[i])
                    / (float) (ir_values[i + 1] - ir_values[i]);
            return ping_values[i] + slope * (raw_ir - ir_values[i]);
        }
    }
    return -1;
}

// Initilization for Ping sensor
void ping_init()
{
    //part 1: Activating the sensor
    //set up wire PB3, enable for GPIO function before trigger pulse
    //Mask Timer Interrupt
    //Send trigger, PB3 set to output
    //Change PB3
    SYSCTL_RCGCGPIO_R |= 0b00000010; //enable port B clock
    SYSCTL_RCGCTIMER_R |= 0x08; //enable clock for timer 3

    GPIO_PORTB_DEN_R |= 0x08; //enable PB3
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFFF0FFF) | 0x00007000; //set PB3 to T3CCP1 function

    //Configure Timer
    TIMER3_CTL_R &= 0x0000; //disable Timer B clear edge mode
    TIMER3_CFG_R = (TIMER3_CFG_R & 0x0) | 0x4; //set Timer 3 to 16 bit mode
    TIMER3_TBMR_R = 0x007; //set Timer3B to count down, edge-time mode, capture mode
    TIMER3_CTL_R |= 0x0C00; //enable for both edges
    TIMER3_TBILR_R |= 0xFFFF; //set timer starting value for counting down
    TIMER3_TBPR_R |= 0xFF; //set prescaler for timer b to extend it to a 24-bit timer
    TIMER3_IMR_R |= 0x400; //enable capture interrupt for Timer3B
    TIMER3_ICR_R |= 0x400; // clear interrupts

    NVIC_EN1_R |= 0x10; //enable TIMER3B interrupts
    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    IntMasterEnable();
    TIMER3_CTL_R |= 0x0100; //enable Timer B
}

// Pulse is sent to the Ping sensor to prime it for a scan
void send_pulse()
{
    TIMER3_CTL_R &= ~0x0100; //disable Timer B
    TIMER3_IMR_R &= 0x000; //mask all interrupts
    GPIO_PORTB_AFSEL_R &= 0xF7; //turn off alternate function for PB3
    GPIO_PORTB_DIR_R |= 0x08; //set PB3 for output

    GPIO_PORTB_DATA_R &= 0xF7; //set PB3 to low
    GPIO_PORTB_DATA_R |= 0x08; //set PB3 to high
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= 0xF7; //set PB3 to low

    // reinitialize for input
    GPIO_PORTB_DIR_R &= 0xF7; //set PB3 for input
    GPIO_PORTB_AFSEL_R |= 0x08; //set PB3 for alternate function

    TIMER3_ICR_R = 0x400; //clear Timer B Capture Mode Event interrupt
    TIMER3_IMR_R |= 0x400; //unmask above interrupt
    TIMER3_CTL_R |= 0x0100; // enable Timer B
}

// Read from the Ping sensor scan
float ping_read()
{
    int cycles;
    float time;
    float distance;

    send_pulse();
    while (!adc_flag) // wait for flag to be set in ISR
    {
    }
    adc_flag = 0; // reset flag
    edge = 1;

    cycles = (ping_sent - ping_received);
    if (cycles < 0)
    {
        overflow_count++;
        cycles += 0xFFFFFF;
    }
    time = (float) cycles / CLOCK_SPEED;
    distance = (time / 2) * 343000; // half the time, multiplied by speed of sound 343000 mm/s

    return distance;
}

// Ping sensor ISR
void TIMER3B_Handler()
{
    if (TIMER3_MIS_R & 0x400)
    {
        TIMER3_ICR_R = 0x400; //clear Timer B Capture Mode Event interrupt
        if (edge == 1)
        {
            ping_sent = TIMER3_TBR_R;
            edge = 0;
        }
        else if (edge == 0)
        {
            ping_received = TIMER3_TBR_R;
            adc_flag = 1;
            edge = -1;
        }
    }
}

//IR calibration method. Overwrites the current IR values, but also sends to putty for saving.
void calibrate_IR()
{
    adc_init();
    ping_init();
    lcd_init();
    servo_init();
    uart_init();

    button_init();
    init_button_interrupts();

    int button;

    servo_move(90);

    char message[100];
    sprintf(message, "\nfloat ping_values[10] = {");
    uart_sendStr(message);

    int i = 0;
    int distance = 10;
    while (distance <= 55)
    {
        button = button_getButton();
        if (button == 4)
        {
            ir_values[i] = adc_IR_read();
            ping_values[i] = ping_read();


            lcd_printf("DIST: %d \nIR: %d \nPING: %.2f", distance, ir_values[i], ping_values[i]);

            i++;
            distance += 5;
            timer_waitMillis(500);
        }
    }

    for (i = 0; i < 10; i++)
    {
        sprintf(message, "%.2f, ", (ping_values[i] / 10));
        uart_sendStr(message);
    }
    uart_sendStr("};\n\r int ir_values[10] = {");
    for (i = 0; i < 10; i++)
    {
        sprintf(message, "%d, ", ir_values[i]);
        uart_sendStr(message);
    }
    uart_sendStr("};\n\r");
}

