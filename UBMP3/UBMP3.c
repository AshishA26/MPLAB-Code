/*==============================================================================
    UBMP 3.0 (PIC16F1459) hardware initialization and user functions.
==============================================================================*/

#include    "xc.h"              // XC compiler general include file

#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions

#include    "UBMP3.h"           // Include UBMP3 constant symbol definitions

// TODO Initialize oscillator, ports and other PIC/UBMP hardware features here.

// Initialize oscillator

void initOsc(void)
{
    OSCCON = 0xFC;              // 3x PLL enabled from 16MHz HFINTOSC
    ACTCON = 0x90;              // Enable active clock tuning from USB
    while(!PLLRDY);             // Wait for PLL lock
}

// Initialize hardware ports and peripherals. Set starting conditions.

void initPorts(void)
{
    OPTION_REG = 0b01010111;    // Enable port pull-ups, TMR0 internal, div-256

    LATA = 0b00000000;          // Clear Port A latches before configuring PORTA
    ANSELA = 0b00000000;        // Make all Port A pins digital
    TRISA = 0b00001111;         // Set RUNLED and Beeper as outputs

    LATB = 0b00000000;          // Clear Port B latches before configuring PORTB
    ANSELB = 0b00000000;        // Make all Port B pins digital
    TRISB = 0b11110000;         // Set Port B pins for switch inputs S2-S5

    LATC = 0b00000000;          // Clear Port C latches before configuring PORTC
    ANSELC = 0b00000000;        // Make all Port C pins digital
    TRISC = 0b00001111;         // Set LED outputs, unused header pins as inputs

    // Enable interrupts, if needed.
}

// Initializes Ch7 analogue input on PORTC. Call initANC after initPorts if
// PORTC analogue input will be used.

void initANC(void)
{
    LATC = 0b00000000;          // Clear Port C latches before configuring PORTC
    ANSELC = 0b00001000;        // Enable RC3 analogue input
    ADCON0 = 0b00011100;        // Analogue channel AN7, A/D converter off,
    ADCON1 = 0b01100000;        // left justified, FOSC/64 A-D clock, +VDD ref.
    ADCON2 = 0b00000000;        // Autoconversion trigger disabled
    TRISC = 0b00001111;         // Set LED outputs, unused header pins as inputs
}

