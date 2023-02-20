/*
 * Part3.c
 *
 *  Created on: Feb 12, 2023
 *      Author: Dylan Wyss
 *
 *      YOU NEED TO FILL IN THIS AUTHOR BLOCK
 */

#define INITIAL_TIMER_VALUE 10000

#include <msp430.h>

int timer_count = INITIAL_TIMER_VALUE;      // Default blinking time value
int timer_state = 0;                        // Determine if LED will blink by default value or value of LED time pressed

void gpioInit();
void timerInit();

void main(){

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    gpioInit();
    timerInit();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    __bis_SR_register(LPM3_bits | GIE);

    while(1){}
}


void gpioInit(){
    // Configure RED LED on P1.0 as Output
    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output direction

    // Configure Button on P2.3 as input with pullup resistor
    P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES &= ~BIT3;                         // P2.3 Low --> High edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

    // Configure Button on P4.1 as input with pullup resistor
    P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
    P4REN |= BIT1;                          // P4.1 pull-up register enable
    P4IES &= ~BIT1;                         // P4.1 Low --> High edge
    P4IE |= BIT1;                           // P4.1 interrupt enabled
}

void timerInit(){
    // Setup Timer Compare IRQ
    TB1CCTL0 |= CCIE;                       // Enable TB1 CCR0 Overflow IRQ
    TB1CTL = TBSSEL_1 | MC_2;               // ACLK, continuous mode
}


/*
 * INTERRUPT ROUTINES
 */

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                         // Clear P2.3 interrupt flag
    if (!(timer_state))
    {
        timer_state = 1;
        timer_count = 0;
        P2IES ^= BIT3;                         // Toggle rising/falling edge
    }
    else
    {
        timer_state = 0;
        P2IES ^= BIT3;                         // Toggle rising/falling edge
    }
}

// Port 4 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                         // Clear P4.1 interrupt flag
    timer_count = INITIAL_TIMER_VALUE;      // Reset timer value to initialized value 300
}

// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    P1OUT ^= BIT0;                          // Toggle Red LED
    TB1CCR0 += timer_count;                 // Increment time between interrupts

    if (timer_state)
    {
        timer_count++;                      // If the button is pressed, continue to count the length
                                            // to add to time of interrupt for LED blinking
    }
}



