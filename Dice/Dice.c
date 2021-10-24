/*==============================================================================
	Project: Dice40
	Version: 2.041				Date: January 17, 2020
	Target: Die2				Processor: PIC12F1840

 An electronic and software-based replacement for a perfectly good plastic cube
 embossed with dots representing the numbers 1-6 on each of its faces.
 
 Pressing the button on the Die2 circuit causes the program to choose a random*
 number between 1 and 6 (with no warranties of its randomness either expressed
 or implied) and, given there is sufficient remaining battery potential, lights
 up that number on an array of LEDs resembling the numbers on a die.
 
 The circuit implements power saving through sleep mode. Pressing the button
 uses an interrupt to wake the device from sleep and generate a new number.
 After displaying the new number for a short period of time, the circuit goes
 back to sleep to save the batteries.
 
 * Note that no random number function is used. The circuit simply counts
 faster than humans while the button is held, and displays the (not so) random,
 but quickly generated, sequential count when the button is released.
 ******************************************************************************/

#include    "xc.h"              // XC compiler general include file

#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions

#include	"Die2.h"			// Include user-created constants and functions

unsigned char count = 1;		// Die count variable
const char period = 50;         // Rolling beep sound period
const char duration = 40;       // Rolling beep sound cycle count
bool rolling = 0;				// Rolling bit. Set to start new die roll.

// Makes beep sound using supplied period and duration.
void beep(unsigned char per, unsigned char dur)
{
	for(dur; dur != 0; dur--)
	{
		BEEPER = !BEEPER;       // Toggle beeper I/O pin
		for(unsigned char i = per; i != 0; i--);    // Period delay
	}
}

// LED display function maps each number to the die display.
void display (unsigned char num)
{
	if (num == 0)
    {
        LED1 = 0;               // All LEDs off
        LED23 = 0;
        LED45 = 0;
        LED67 = 0;
    }
	else if (num == 1)
    {
        LED1 = 1;               // Centre LED on
        LED23 = 0;
        LED45 = 0;
        LED67 = 0;
    }
	else if (num == 2)
    {
        LED1 = 0;
        LED23 = 1;              // LED2 and LED3 on
        LED45 = 0;
        LED67 = 0;
    }
	else if (num == 3)
    {
        LED1 = 1;
        LED23 = 0;
        LED45 = 0;
        LED67 = 1;
    }
	else if (num == 4)
    {
        LED1 = 0;
        LED23 = 1;
        LED45 = 0;
        LED67 = 1;
    }
	else if (num == 5)
    {
        LED1 = 1;
        LED23 = 1;
        LED45 = 0;
        LED67 = 1;
    }
	else if (num == 6)
    {
        LED1 = 0;
        LED23 = 1;
        LED45 = 1;
        LED67 = 1;
    }
    else
    {
        LED1 = 0;
        LED23 = 0;
        LED45 = 0;
        LED67 = 0;
    }
}

// Interrupt service routine. S1 button wakes microcontroller from sleep.

void __interrupt() wake(void)
{
    di();                       // Disable interrupts
	if(IOCIF == 1 && IOCIE == 1)	// Check for IOC interrupt
	{
        IOCAF = 0;              // Clear IOC port A interrupt flag
        IOCIF = 0;				// Clear IOC interrupt flag
		rolling = 1;			// Send a die rolling message to main
	}
	else
    {
        IOCAN = 0b00001000;     // Other interrupt? Only allow GPIO IOC change
		INTCON = 0b00001000;
    }
	return;
}

// Start of main program code.

int main(void)
{
	initPorts();				// Initialize I/O Ports
	LATA = 0b00010111;			// Test LEDs - all on
	beep(160,50);				// Make power-up sound
	__delay_ms(20);
	beep(80,80);
	__delay_ms(250);
	LATA = 0;					// LEDs off
	
	while(1)                    // Main program loop
	{
		while(rolling == 1)		// New roll?
		{
			while(S1 == 0)      // While button is held...
			{
				count++;		// Roll die
				if(count > 6)
					count = 1;
                display(count); // Display count
                beep(period, duration); // Make beeping sound
                __delay_us(500);        // A short pause
			}
			
            // After button is released, count slows down..
            
			for(unsigned char i = 0; i != 200; i+=20)
			{
				count++;		// Keep rolling die
				if(count > 6)
					count = 1;
                display(count); // Display count
                beep(period+i, duration+i); // Increase beep period and length
                __delay_ms(20);
			}

            // After count stops, last count is displayed on LEDs
            
			__delay_ms(2500);   // Wait before shutting down
            rolling = 0;        // End die roll
		}

		// Get ready to sleep
        display(0);             // Turn all LEDs off
        IOCAF = 0;              // Clear interrupt on change pin flag
		IOCIF = 0;				// Clear global interrupt on change flag
		IOCIE = 1;				// Enable interrupt on change interrupt
		ei();                   // Enable global interrupts
		SLEEP();                // Sleep until button is pressed
	}
}

