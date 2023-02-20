/*
 * Part2.c
 *
 *  Created on: Feb 11, 2023
 *      Author: Russell Trafford
 *      Editor: Dylan Wyss
 *
 *      This code will need to change the speed of an LED between 3 different speeds by pressing a button.
 */

#include <msp430.h>

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

    while(1){}
}


void gpioInit(){
    // @TODO Initialize the Red or Green LED
    // Configure RED LED on P1.0 as Output
    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output direction

    // @TODO Initialize Button 2.3
    // Configure Button on P2.3 as input with pullup resistor
    P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES &= ~BIT3;                         // P2.3 Low --> High edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

}

void timerInit(){
    // @TODO Initialize Timer B1 in Continuous Mode using ACLK as the source CLK with Interrupts turned on
    // Timer1_B3 setup
    TB1CTL |= TBCLR;                        // Clear timer and dividers
    TB1CTL |= TBSSEL__SMCLK;                // Source = SMCLK
    TB1CTL |= MC__CONTINUOUS;               // Mode = Continuous

    // Setup Timer Compare IRQ
    TB1CCTL0 |= CCIE;                       // Enable TB1 CCR0 Overflow IRQ
    TB1CCR0 = 50000;
    TB1CTL = TBSSEL_1 | MC_2;               // ACLK, continuous mode
    TB1CCTL0 &= ~CCIFG;                     // Clear CCR0 Flag
}


/*
 * INTERRUPT ROUTINES
 */

// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    // @TODO Remember that when you service the GPIO Interrupt, you need to set the interrupt flag to 0.
    if (P2IES & BIT3)
        TB1CTL &= ~TBIFG;                       // Clear TB1 flag
    else
        TB1CTL |= TBIFG;

    // @TODO When the button is pressed, you can change what the CCR0 Register is for the Timer. You will need to track what speed you should be flashing at.
    if (!(P2IN & BIT3))
        if (TB1CCR0 < 150001)
            TB1CCR0 += 50000;                       // Add offset to TB1CCR0
        else
            TB1CCR0 = 50000;                        // Reset offset to 50000
    else
        TB1CCR0 = TB1CCR0;
}


// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    // @TODO You can toggle the LED Pin in this routine and if adjust your count in CCR0.
    P1OUT ^= BIT0;                          // Toggle Red LED
    TB1CCTL0 &= ~CCIFG;                     // Clear CCR0 Flag
}


