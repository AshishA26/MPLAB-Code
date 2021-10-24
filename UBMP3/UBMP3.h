/*==============================================================================
	UBMP 3.0 (PIC16F1459) symbolic constant definitions.
==============================================================================*/

// TODO - Add user constant definitions for UBMP hardware here.

// Inputs read the port registers (eg. RC0). Outputs write to the port latches
// (eg. LATC0). Create two different symbolic names for any pin that will be
// used for both input and output.

// PORTA I/O device definitions

#define S1          RA3         // S1/Reset switch input
#define BEEPER      LATA4       // Piezo beeper P1 output
#define	P1          LATA4       // Piezo beeper P1 output
#define LED1        LATA5       // LED1/Run LED output (active-low)
#define RUNLED      LATA5       // LED1/Run LED output (active-low)
#define LED2        LATA5       // LED2/IR LED output (active-high)
#define IRLED       LATA5       // LED2/IR LED output (active-high)

// PORTB I/O device definitions

#define S2          RB4         // Switch S2 input
#define S3          RB5         // Switch S3 input
#define S4          RB6         // Switch S4 input
#define S5          RB7         // Switch S5 input

// PORTC I/O device definitions

#define H1IN        RC0         // External I/O header H1 input
#define H1OUT       LATC0       // External I/O header H1 output
#define H2IN        RC1         // External I/O header H2 input
#define H2OUT       LATC1       // External I/O header H2 output
#define H3IN        RC2         // External I/O header H3 input
#define H3OUT       LATC2       // External I/O header H3 output 
#define H4IN        RC3         // External I/O header H4 input
#define IR          RC3         // IR demodulator U2 input
#define U2          RC3         // IR demodulator U2 input
#define Q1          RC3         // Ambient light sensor/phototransistor Q1 input
#define H4OUT       LATC3       // External I/O header H4 output
#define H5IN        RC4         // External I/O header H5 input
#define LED3        LATC4       // LED3 output
#define H5OUT       LATC4       // External I/O header H5 output
#define H6IN        RC5         // External I/O header H6 input
#define LED4        LATC5       // LED4 output
#define H6OUT       LATC5       // External I/O header H6 output
#define H7IN        RC6         // External I/O header H7 input
#define LED5        LATC6       // LED5 output
#define H7OUT       LATC6       // External I/O header H7 output
#define H8IN        RC7         // External I/O header H8 input
#define LED6        LATC7       // LED6 output
#define H8OUT       LATC7       // External I/O header H8 output

// A-D Converter input channel definitions

#define AN4         0b00010000  // A-D channel 4 input
#define ANH1        0b00010000  // External H1 header analogue input (Ch4))
#define AN5         0b00010100  // A-D channel 5 input
#define ANH2        0b00010100  // External H2 header analogue input (Ch5)
#define AN6         0b00011000  // A-D channel 6 input
#define ANH3        0b00011000  // External H3 header analogue input (Ch6)
#define AN7         0b00011100  // A-D channel 7 input
#define ANH4        0b00011100  // External H4 header analogue input (Ch7)
#define ANQ1        0b00011100  // Q1 phototransistor A-D input (Ch7)
#define AN8         0b00100000  // A-D channel 8 input
#define ANH7        0b00100000  // External H7 header analogue input (Ch8)
#define AN9         0b00100100  // A-D channel 9 input
#define ANH8        0b00100100  // External H8 header analogue input (Ch9)
#define AN10        0b00101000  // A-D channel 10 input (S2 pushbutton)
#define AN11        0b00101100  // A-D channel 11 input (S3 pushbutton)
#define ADTIM       0b01110100  // PICmicro on-die temperature indicator module

// Clock frequency for delay macros and simulation

#define _XTAL_FREQ  48000000    // Set clock frequency for time delays

// TODO - Add function prototypes for all functions in UBMP3.c here.

void initOsc(void);             // Oscillator initialization function prototype.
void initPorts(void);           // Port initialization function prototype.
void initANC(void);             // Analogue PORTC initialization function
