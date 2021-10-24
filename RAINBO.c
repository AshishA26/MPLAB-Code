/*==============================================================================
	Project: RAINBO2-3pix
	Version: 2.0				Date: Nov 8, 2019
	Target: RAINBO2				Processor: PIC12F1840

 RAINBO board NeoPixel driver for 3 NeoPixels with touch button support.
 =============================================================================*/

#include    "xc.h"              // XC compiler general include file

#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions

#include	"RAINBO2.h"			// Include user-created constants and functions

// Touch sensor/function variables

unsigned int touchCalcAvg;      // Temporary variable to initialize touch average
unsigned char touchCount;       // Touch sensor CPS oscillator cycle count
unsigned char touchAverage;     // Touch sensor average count
unsigned char touchTrip;        // Touch sensor trip point
unsigned char touchDelta;       // Touch count difference from Tavg
unsigned char touchActive;      // Number of active touch targets (0 = none)

// NeoPixel constants

#define maxLEDs 9               // Number of LEDs in the string
#define delLEDs 25              // Delay between LED update cycles in ms

// NeoPixel RGB byte variables

unsigned char red = 160;        // RGB colour bytes and default starting colour
unsigned char green = 32;
unsigned char blue = 0;

//unsigned char redNP[maxLEDs];   // RGB colour arrays
//unsigned char greenNP[maxLEDs];
//unsigned char blueNP[maxLEDs];
unsigned char redNP[9] = {128,255,255,128,128,0,0,0,0};
unsigned char greenNP[9] = {0,0,128,255,255,128,128,0,0};
unsigned char blueNP[9] = {0,0,0,0,128,255,255,128,128};
    

unsigned char ri;               // RGB colour index for array/table reads
unsigned char gi;
unsigned char bi;

// Generic variables

unsigned char i;				// Generic index counter
unsigned char temp;             // Generic temporary variable

// Declarations for external random number functions
extern int rand();				// External random number function
extern void srand(unsigned int); // External random seed function
//unsigned int newSeed;			// Random seed value

const char sine[180] = {        // 120 position 8-bit sine wave look-up table
0, 0, 1, 2, 3, 4, 6, 9,
11, 14, 17, 21, 24, 29, 33, 37,
42, 47, 53, 58, 64, 70, 76, 82,
88, 95, 101, 108, 115, 121, 128, 134,
140, 147, 154, 160, 167, 173, 179, 185,
191, 197, 202, 208, 213, 218, 222, 226,
231, 234, 238, 241, 244, 246, 249, 251,
252, 253, 254, 255, 255, 255, 254, 253,
252, 251, 249, 246, 244, 241, 238, 234,
231, 226, 222, 218, 213, 208, 202, 197,
191, 185, 179, 173, 167, 160, 154, 147,
140, 134, 127, 121, 115, 108, 101, 95,
88, 82, 76, 70, 64, 58, 53, 47,
42, 37, 33, 29, 24, 21, 17, 14,
11, 9, 6, 4, 3, 2, 1, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0};

/*==============================================================================
 *	anConvert - Perform analogue to digital conversion with input from selected
 *              channel. See channel constants in the project's .h file.
 *============================================================================*/

unsigned char anConvert(unsigned char chan)
{
	ADON = 1;					// Turn A-D converter on
	ADCON0 = (ADCON0 & 0b10000011);	// Change the conversion channel by wiping
	ADCON0 = (ADCON0 | chan);		// CHS bits and ORing with selected channel
	__delay_us(2);				// Give input time to settle
	GO = 1;						// Start the conversion

	while(GO);					// Wait for the conversion to finish (GO=0)

	ADON = 0;					// Turn A-D converter off and
	return (ADRESH);			// return the upper 8-bits (MSB) of the result
}

/*==============================================================================
 *	touchInit - Initialize touch sensor resting average before sensing touch.
 *============================================================================*/

void touchInit(void)
{
    touchCalcAvg = 0;			// Reset temporary average accumulator
    for(unsigned char c = 16; c != 0; c--)  // Take 16 samples
    {
        TMR0 = 0;				// Clear capacitive oscillator timer
        __delay_ms(1);			// Wait for fixed sensing time-base
        touchCalcAvg+= TMR0;	// Add capacitor oscillator count to temp
    }
    touchAverage = touchCalcAvg / 16;   // Save average of 16 cycles
}

/*==============================================================================
 *	touch - Simple touch input function. Returns 1 on touch, 0 for no touch.
 *============================================================================*/

bool touch(void)
{
    TMR0 = 0;                   // Clear cap oscillator cycle timer
    __delay_us(1000);           // Wait for fixed sensing time-base
    touchCount = TMR0;          // Save current oscillator cycle count
    touchTrip = touchAverage / 4;   // Set trip point -25% below average
    if(touchCount < (touchAverage - touchTrip)) // Is T1 tripped?
    {
        return(1);              // Yes, return 1.
    }
    else                        // No, check average limit before returning 0.
    {
        if(touchCount > touchAverage)   // Average < count?
        {
            touchAverage = touchCount;  // Bump average to prevent future underflow
        }
        else                    // Or, calculate new average
        {
            touchAverage = touchAverage - (touchAverage / 16) + (touchCount / 16);                    
        }
        return(0);
    }
}

/*==============================================================================
 *	neoRGB - sets requested number of Neopixel LEDs to same RGB value.
 =============================================================================*/

void neoRGB(unsigned char red, unsigned char green, unsigned char blue, unsigned char leds)
{
    unsigned char b;                // Bit counter for each 8-bit colour byte
    
	for(leds; leds != 0; leds--)	// Repeat all 24 colour bits for each LED
	{
		temp = green;               // Copy green value before shifting MSB first
		for(b = 8; b != 0; b--)		// PWM each bit in assembly code for speed
		{
			asm("bsf LATA,5");		// Make N2 output high
			asm("nop");				// Wait one instruction cycle
			asm("btfss _temp,7");	// If MSB == 1, skip next instruction
			asm("bcf LATA,5");		// Make N2 output low (0)
			asm("lslf _temp,f");	// Shift next bit into MSB position
			asm("bcf LATA,5");		// Make N2 output low (1)
		}
		temp = red;					// Red next, same as green.
		for(b = 8; b != 0; b--)
		{
			asm("bsf LATA,5");
			asm("nop");
			asm("btfss _temp,7");
			asm("bcf LATA,5");
			asm("lslf _temp,f");
			asm("bcf LATA,5");
		}
		temp = blue;                // Blue last. 
		for(b = 8; b != 0; b--)
		{
			asm("bsf LATA,5");
			asm("nop");
			asm("btfss _temp,7");
			asm("bcf LATA,5");
			asm("lslf _temp,f");
			asm("bcf LATA,5");
		}
	}
}

void neoRGBArray(unsigned char leds)
{
    unsigned char b;                // Bit counter for each 8-bit colour byte
    unsigned char led;              // Current LED count
    
	for(led = 0; led != leds; led++) // Repeat all 24 colour bits for each LED
	{
		temp = greenNP[led];        // Copy green value before shifting MSB first
		for(b = 8; b != 0; b--)		// PWM each bit in assembly code for speed
		{
			asm("bsf LATA,5");		// Make N2 output high
			asm("nop");				// Wait one instruction cycle
			asm("btfss _temp,7");	// If MSB == 1, skip next instruction
			asm("bcf LATA,5");		// Make N2 output low (0)
			asm("lslf _temp,f");	// Shift next bit into MSB position
			asm("bcf LATA,5");		// Make N2 output low (1)
		}
		temp = redNP[led];          // Red next, same as green.
		for(b = 8; b != 0; b--)
		{
			asm("bsf LATA,5");
			asm("nop");
			asm("btfss _temp,7");
			asm("bcf LATA,5");
			asm("lslf _temp,f");
			asm("bcf LATA,5");
		}
		temp = blueNP[led];            // Blue last. 
		for(b = 8; b != 0; b--)
		{
			asm("bsf LATA,5");
			asm("nop");
			asm("btfss _temp,7");
			asm("bcf LATA,5");
			asm("lslf _temp,f");
			asm("bcf LATA,5");
		}
	}
}

/*==============================================================================
 *	main - Main program function with initialization code and forever loop.
 =============================================================================*/

int main(void)					// Start of program
{
	init();						// Initialize oscillator and I/O Ports
    touchInit();                // Initialize touch sensor average count
    
    N1 = 0;                     // Clear NeoPixel output and force a reset
    __delay_us(200);            // pulse duration to initialize NeoPixel string

    neoRGBArray(9);
    SLEEP();
    
    while(1)
    {
        red=255;
        green=255;
        blue=255;
        while(red > 0)          // Always check the brightest value
        {
            neoRGB(red, green, blue, 1);
            __delay_ms(10);
            red=red/2;
            green=green/2;
            blue=blue/2;
        }
        neoRGB(red, green, blue, 1);
    
        __delay_ms(1000);
    }
    
//    ri = 60;
//    gi = 0;
//    bi = 120;
//    
//    while(1)                    // Code in this loop repeats forever
//    {
//        neoRGB(sine[ri], sine[gi], sine[bi], maxLEDs);  // Turn on all LEDs
//        __delay_ms(delLEDs);    // Wait a bit
//        
//        if(touch() == 1)
//        {
//            ri = ri + 1;
//            if(ri == 180)
//                ri = 0;
//
//            bi = bi + 1;
//            if(bi == 180)
//                bi = 0;
//
//            gi = gi + 1;
//            if(gi == 180)
//                gi = 0;
//        }
//    }
    
}
