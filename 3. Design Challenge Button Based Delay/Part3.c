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

unsigned long count_timer = 0;             // Default blinking time value
unsigned int counting = 0;                  // Determine if LED will blink by default value or value of LED time pressed
                                            // 0 = timer not counting, 1 = timer counting
unsigned int rising_edge = 1;
unsigned int falling_edge = 0;

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
    __no_operation();                             // For debug
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
    TB0CCTL0 |= CCIE;                       // Enable TB0 CCR0 Overflow IRQ
    TB0CCR0 = 1;
    TB0CTL = TBSSEL_1 | MC_2;               // ACLK, continuous mode

    // Setup Timer Compare IRQ
    TB1CCTL0 |= CCIE;                       // Enable TB1 CCR0 Overflow IRQ
    TB1CCR0 = INITIAL_TIMER_VALUE;
    TB1CTL = TBSSEL_1 | MC_2;               // ACLK, continuous mode
}


/*
 * INTERRUPT ROUTINES
 */

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                            // Clear P2.3 interrupt flag
    if (rising_edge)
    {
        rising_edge = 0;
        falling_edge = 1;
        P2IES &= ~BIT3;                        // P2.3 Low --> High edge
        counting = 1;
        count_timer = 0;
    }
    else if (falling_edge)
    {
        rising_edge = 1;
        falling_edge = 0;
        P2IES |= BIT3;                         // P2.3 High --> Low edge
        counting = 0;
    }
}

// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                         // Clear P4.1 interrupt flag
    count_timer = INITIAL_TIMER_VALUE;      // Reset timer value to initialized value 10000
    counting = 0;
}

// Timer B0 interrupt service routine
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
{
    if (counting)
        count_timer++;                      // If the button is pressed, continue to count the length
                                            // to add to time of interrupt for LED blinking
    else
        count_timer = count_timer;
}

// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    P1OUT ^= BIT0;                          // Toggle Red LED
    TB1CCR0 += count_timer;                 // Increment time between interrupts
}

