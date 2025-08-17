/*
 *
 *   uart.c
 *
 *
 *
 *
 *
 *   @author
 *   @date
 */

#include "uart.h"

void uart_init(void)
{
    SYSCTL_RCGCGPIO_R |= 0x2;        // enable clock for Port B
    timer_waitMillis(1); // Small delay before accessing device after turning on clock
    SYSCTL_RCGCUART_R |= 0x2;        // enable clock UART1 (page 344)
    GPIO_PORTB_AFSEL_R |= 0x03;      // set pins PB0 and PB1 for alternate func.
    GPIO_PORTB_PCTL_R &= 0xFFFFFF00; // Force 0's for PB0 and PB1
    GPIO_PORTB_PCTL_R |= 0x00000011; // Force 1's to select U1Rx and U1Tx in PB0 and PB1
    GPIO_PORTB_DEN_R |= 0x03;        // enable PBO and PB1
    GPIO_PORTB_DIR_R &= 0xFD;        // Force 0 for PB1 as input
    GPIO_PORTB_DIR_R |= 0x01;        // Force 1 for PB0 as output

    // UART clock 16MHz
    double baud_rate = 115200;
    double BRD = 16000000 / (16 * baud_rate); // assuming ClkDiv = 16, High speed is off
    int IBRD = (int) BRD;
    double FBRD = BRD - IBRD;
    FBRD = (int) (FBRD * 64 + 0.5);

    UART1_CTL_R = UART1_CTL_R & 0xFFFE;          // disable UART1, clear bit 0
    UART1_IBRD_R = IBRD;            // integer portion of BRD to IBRD
    UART1_FBRD_R = FBRD;            // fractional portion of BRD to FBRD
    UART1_LCRH_R = (UART1_LCRH_R & 0b11110101) | 0b01100000; // serial communication parameters (page 916) * 8bit and no parity, clear bits 1 and 3, set 6:5 = 0b11
    UART1_CC_R &= 0x0; // use system clock as clock source (page 939), clear 3:0
    UART1_CTL_R |= 0x0301;          // enable UART1 and transmit and receive

}

// transmits char from UART
void uart_sendChar(char data)
{
    while (UART1_FR_R & 0x20)
    {
    }
    UART1_DR_R = data;
}

// Receives char from UART
char uart_receive(void)
{
    // UART1_FR_R & 0x10
    while (flag == 0)
    {
    }
    flag = 0;
    // uart_data = (char) (UART1_DR_R & 0xFF); // store the last 8 bits of UART1 DATA
    return uart_data;
}

void uart_sendStr(const char *data)
{
    while (*data != '\0')
    {
        uart_sendChar(*data);
        data++;
    }
}

char uart_receive_nonblocking() {
    char data;
    data = uart_data;
    uart_data = 0;
    return data;
}

void uart_interrupt_init()
{

    UART1_CTL_R = UART1_CTL_R & 0xFFFE;          // disable UART1, clear bit 0
    UART1_ICR_R |= 0x10; // clear interrupt
    UART1_IM_R |= 0x10;   // unmask bit 4, UART receive interrupt
    NVIC_PRI1_R |= 0x00200000; //sets priority of usart1 interrupt
    NVIC_EN0_R |= 0x40; // set bit 6 for enabling UART1 interrupt
    IntRegister(INT_UART1, uart_interrupt_handler);
    IntMasterEnable();
    UART1_CTL_R |= 0x0301;          // enable UART1 and transmit and receive
}

void uart_interrupt_handler()
{
    // check if unmasked interrupt has occurred, bit 4
    if (UART1_MIS_R & 0x10)
    {
        flag = 1;    //set flag
        uart_data = (char) UART1_DR_R & 0xFF; // store the last 8 bits of UART1 DATA
        UART1_ICR_R |= 0x10;            // clear interrupt
    }
}

