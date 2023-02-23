# Design Challenge: Button-Based Delay
The code implemented for this design challenge is as follows:

The program begins by including any necessary libraries, functions, and variables that will be used later on in the code.
```c
#define INITIAL_TIMER_VALUE 10000

#include <msp430.h>

unsigned long count_timer = 0;             // Default blinking time value
unsigned int counting = 0;                  // Determine if LED will blink by default value or value of LED time pressed
                                            // 0 = timer not counting, 1 = timer counting
unsigned int rising_edge = 1;
unsigned int falling_edge = 0;

void gpioInit();
void timerInit();
```

The above variables need to be explained. 'count_timer' is the variable that holds information for how long the button has been pressed to then impact how long the LED blinks for. 'counting' declares true or false for if the timer should count how long the button has been held, and is true if the button has been pressed. 'rising_edge' is a true/false function that returns if the button is pressed on a rising clock edge. Likewise, 'falling_edge' is the same but for a falling edge.

Inside the main function, the watchdog timer is configured, the functions are called, power-on mode is set to high, and the interrupt is called out.
```c
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
```

Next up the peripherals need to be initialized. The red LED (P1.0) is initialized to a power-on state and is set to be an output. Both buttons (P2.3 and P4.1) are initialized to have a pull-up resistor where the interrupt is enabled going from the high to low edge of the clock cycle.
```c
void gpioInit(){
    // Configure RED LED on P1.0 as Output
    P1OUT &= ~BIT0;                         // Clear P1.0 output latch for a defined power-on state
    P1DIR |= BIT0;                          // Set P1.0 to output direction

    // Configure Button on P2.3 as input with pullup resistor
    P2OUT |= BIT3;                          // Configure P2.3 as pulled-up
    P2REN |= BIT3;                          // P2.3 pull-up register enable
    P2IES |= BIT3;                         // P2.3 High --> Low edge
    P2IE |= BIT3;                           // P2.3 interrupt enabled

    // Configure Button on P4.1 as input with pullup resistor
    P4OUT |= BIT1;                          // Configure P4.1 as pulled-up
    P4REN |= BIT1;                          // P4.1 pull-up register enable
    P4IES |= BIT1;                         // P4.1 High --> Low edge
    P4IE |= BIT1;                           // P4.1 interrupt enabled
}
```

Next up is the initialization of the timers. There are two timers used for this project: Timer B0 and Timer B1. Both timers are configured to enable overflow and are initialized to use the asynchronous clock (ACLK) in continuous mode. The only difference between these two initializations is that TB0 is set to the frequency of 1 Hz for the capture control register as it will count up if the button is held for every instance. However, TB1 is set to the previously defined initial value of 50000 Hz.
```c
void timerInit(){
    // Setup Timer Compare IRQ
    TB0CCTL0 |= CCIE;                       // Enable TB0 CCR0 Overflow IRQ
    TB0CCR0 = 1;
    TB0CTL = TBSSEL_1 | MC_2 | ID_3;        // ACLK, continuous mode

    // Setup Timer Compare IRQ
    TB1CCTL0 |= CCIE;                       // Enable TB1 CCR0 Overflow IRQ
    TB1CCR0 = INITIAL_TIMER_VALUE;
    TB1CTL = TBSSEL_1 | MC_2 | ID_3;        // ACLK, continuous mode
}
```

For the interrupt service routine for button 2.3, the interrupt flag is cleared as the interrupt has been instantiated. If the interrupt occurs on a rising edge, it is then set to a falling edge, the LED stops lighting up, and the timer begins to count how long the button has been held being from the intialized value of 0. Meanwhile, if the interrupt occurs on a fallign edge, it is set to a rising edge.
```c
// Port 2 interrupt service routine
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    P2IFG &= ~BIT3;                            // Clear P2.3 interrupt flag
    if (rising_edge)
    {
        rising_edge = 0;
        falling_edge = 1;
        P1OUT &= ~BIT0;                        // set red LED to low output
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
```

Meanwhile, if the other button (4.3) is pressed, the flag is cleared and the LED returns to blinking the initial value.
```c
// Port 4 interrupt service routine
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
{
    P4IFG &= ~BIT1;                         // Clear P4.1 interrupt flag
    TB1CCR0 = INITIAL_TIMER_VALUE;
    counting = 0;
}
```

The first timer counts how long the button has been held. If counting is true, then the value 'count_timer' is incremented. If not, then this value remains the same.
```c
// Timer B0 interrupt service routine
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer0_B0_ISR(void)
{
    if (counting)
        count_timer++;                      // If the button is pressed, continue to count the length
                                            // to add to time of interrupt for LED blinking
    else
        count_timer = count_timer;
    TB0CCR0 += 1;                           // Add offset to TB0CCR0
}
```

Lastly, the second timer toggles the LED on and off for the duration that has been set up incrementing the capture control register by the 'count_timer' value which previous counted how long the button was held for.
```c
// Timer B1 interrupt service routine
#pragma vector = TIMER1_B0_VECTOR
__interrupt void Timer1_B0_ISR(void)
{
    P1OUT ^= BIT0;                          // Toggle Red LED
    TB1CCR0 += count_timer;                 // Increment time between interrupts
}
```

This code can be used to demonstrate the red LED blinking for the duration that button 2.3 was pressed, as intended.
