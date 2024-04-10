////////////////////////////////////////////////////////////////////////////////
//
// Main
//
// If SW2 is 0 then pressing SW3 will turn on the red LED and pressing SW3 will
// turn on the green LED.
// If SW2 is 1 then pressing SW3 will turn off the red LED and pressing SW3 will
// turn off the green LED.
//
////////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <avr/interrupt.h>

// Macros for LEDs
#define LED0_DDR   DDRB
#define LED0_PORT  PORTB
#define LED0_INDEX PIN0

#define LED1_DDR   DDRB
#define LED1_PORT  PORTB
#define LED1_INDEX PIN5


// Macros for switch & pushbuttons
#define SW2_DDR     DDRD
#define SW2_PORT    PORTD   //config pull-up
#define SW2_RD      PIND
#define SW2_INDEX   PIN2
//set an internal pull-up resistor?

#define SW3_DDR     DDRB
#define SW3_PORT    PORTB   //config pull-up
#define SW3_RD      PINB
#define SW3_INDEX   PIN6

#define SW4_DDR     DDRD
#define SW4_PORT    PORTD   //config pull-up
#define SW4_RD      PIND
#define SW4_INDEX   PIN7


// Macros for motors
#define LEFT1_DDR      DDRD
#define LEFT1_PORT     PORTD
#define LEFT1_INDEX    PIN3
#define LEFT1_OCR      OCR2B

#define LEFT2_DDR      DDRB
#define LEFT2_PORT     PORTB
#define LEFT2_INDEX    PIN3


#define RIGHT1_DDR      DDRD
#define RIGHT1_PORT     PORTD
#define RIGHT1_INDEX    PIN6
#define RIGHT1_OCR      OCR0A

#define RIGHT2_DDR      DDRD
#define RIGHT2_PORT     PORTD
#define RIGHT2_INDEX    PIN5


#define DISPENSER1_DDR      DDRB
#define DISPENSER1_PORT     PORTB
#define DISPENSER1_INDEX    PIN1
#define DISPENSER1_OCR      OCR1A

#define DISPENSER2_DDR      DDRB
#define DISPENSER2_PORT     PORTB
#define DISPENSER2_INDEX    PIN2


int main(void) {

    int sw2State, sw3State, sw4State;

    // Configure the LED pins as outputs
    LED0_DDR |= _BV(LED0_INDEX);
    LED1_DDR |= _BV(LED1_INDEX);

    // Configure the motor pins as outputs
    LEFT1_DDR       |= _BV(LEFT1_INDEX);
    LEFT2_DDR       |= _BV(LEFT2_INDEX);
    RIGHT1_DDR      |= _BV(RIGHT1_INDEX);
    RIGHT2_DDR      |= _BV(RIGHT2_INDEX);
    DISPENSER1_DDR  |= _BV(DISPENSER1_INDEX);
    DISPENSER2_DDR  |= _BV(DISPENSER2_INDEX);

    // Initialize both LEDs as off
    LED0_PORT |= _BV(LED0_INDEX);
    LED1_PORT |= _BV(LED1_INDEX);

    // Initialize outputs as off
    LEFT1_PORT      &= ~_BV(LEFT1_INDEX);
    LEFT2_PORT      |= _BV(LEFT2_INDEX);
    RIGHT1_PORT     &= ~_BV(RIGHT1_INDEX);
    RIGHT2_PORT     &= ~_BV(RIGHT2_INDEX);
    DISPENSER1_PORT &= ~_BV(DISPENSER1_INDEX);
    DISPENSER2_PORT &= ~_BV(DISPENSER2_INDEX);

    // Configure the switch & pushbuttons as inputs
    SW2_DDR &= ~_BV(SW2_INDEX);
    SW3_DDR &= ~_BV(SW3_INDEX);
    SW4_DDR &= ~_BV(SW4_INDEX);

    // Configure the switch & pushbuttons' pull-up resistors
    SW2_PORT |=  _BV(SW2_INDEX);    // Turn on pull-up resistor
    SW3_PORT &= ~_BV(SW3_INDEX);    // Turn off pull-up resistor
    SW4_PORT &= ~_BV(SW4_INDEX);    // Turn off pull-up resistor


    while(1)
    {
        sw2State = (SW2_RD & _BV(SW2_INDEX)) >> SW2_INDEX;
        sw3State = (SW3_RD & _BV(SW3_INDEX)) >> SW3_INDEX;
        sw4State = (SW4_RD & _BV(SW4_INDEX)) >> SW4_INDEX;


        if (sw4State == 1) {
            DISPENSER1_PORT |= _BV(DISPENSER1_INDEX);
            DISPENSER2_PORT &= ~_BV(DISPENSER2_INDEX);
        }
        else if (sw3State == 1) {
            DISPENSER1_PORT &= ~_BV(DISPENSER1_INDEX);
            DISPENSER2_PORT |= _BV(DISPENSER2_INDEX);
        }
        else {
            DISPENSER1_PORT &= ~_BV(DISPENSER1_INDEX);
            DISPENSER2_PORT &= ~_BV(DISPENSER2_INDEX);
        }
    }

    // Loop forever at the end of main
    while(1);

}