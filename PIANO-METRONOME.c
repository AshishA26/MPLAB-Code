/*==============================================================================
	Project: PIANO-MET
	Version: 1.0				Date: January 22, 2020
	Target: PIANO2  			Processor: PIC12F1840

  Piano and Metronome program for the PIANO2 board. Press S1 to enable piano
  and tap keys. Pressing S1 again switches to metronome mode. Pressing S1 again
  puts Piano into a low-power mode.
  
  If the piano will not be used for a period of time, remove the batteries.

  Currently sleeps and uses Watch Dog timer for periodic wake-up. Next version
  try shutdown with button wake/restart.
 =============================================================================*/

#include    "xc.h"              // XC compiler general include file

#include    "stdint.h"          // Include integer definitions
#include    "stdbool.h"         // Include Boolean (true/false) definitions

#include	"PIANO2.h"          // Include hardware constants and functions

// Capacitive Sensing Module (CPS)/Touch sensor variables and threshold

unsigned int Ttemp;             // Temporary variable to initialize touch averages
unsigned char Tcount[4];		// CPS oscillator cycle counts for each touch sensor
unsigned char Tavg[4];			// Average count for each touch sensor
unsigned char Ttrip[4];			// Trip point for each touch sensor
unsigned char Tdelta[4];		// Difference of touch from average sensor count
const char Tthresh = 4;         // Sensor active threshold (below Tavg)


// Touch processing variables
unsigned char Tactive;			// Number of active touch targets (0 = none)
unsigned char Ttarget[4];       // Positions of active touch targets
unsigned char note = 0;         // Current note

// Operating mode constants
#define off 0
#define piano 1
#define metronome 2

bool modeSwitch = false;        // Mode switch in progress
unsigned char mode = piano;     // Current operating mode

// Metronome variables
bool beatOn = true;             // Metronome beating
bool settingChange = false;     // Key toggle boolean for setting changes
unsigned char beat = 0;         // Current beat count
unsigned char beats = 1;        // Beats per measure
unsigned char bpm = 60;         // Starting metronome frequency
unsigned char bpmIndex;         // Index to beatDelay table

const int beatDelay[41] = {     // Delays for bpm values from 40 to 240
1500,1333,1200,1091,1000,923,857,800,
750,706,667,632,600,571,545,522,
500,480,462,444,429,414,400,387,
375,364,353,343,333,324,316,308,
300,293,286,279,273,267,261,255,
250 };

/*==============================================================================
 *	makeBeat - Creates metronome beats.
 *============================================================================*/

void makeBeat(unsigned int counts)
{
    if(beat == 0)                  // First beat is high note
    {
        TMR2ON = 1;
        PR2 = 68;
        CCPR1L = 34;
        __delay_ms(20);
        TMR2ON = 0;
    }
    else                            // Subsequent beets are low notes
    {
        TMR2ON = 1;
        PR2 = 136;
        CCPR1L = 68;
        __delay_ms(20);
        TMR2ON = 0;
    }
    beat++;                         // Increment beat counter
    if(beat == beats)
    {
        beat = 0;
    }
    for(counts; counts != 0; counts --) // Count bpm delay
    {
        __delay_us(990);
    }
}

void initTouch(void)
{
	for(unsigned char i = 0; i != 4; i++)
	{
		CPSCON1 = i;				// Sense each of the 4 touch sensors in turn
		Ttemp = 0;					// Reset temporary counter
		for(unsigned char c = 16; c != 0; c--)
		{
			TMR0 = 0;				// Clear capacitive oscillator timer
			__delay_ms(1);			// Wait for fixed sensing time-base
			Ttemp += TMR0;			// Add capacitor oscillator count to temp
		}
		Tavg[i] = Ttemp / 16;		// Save average of 16 cycles
	}
}

/*==============================================================================
 *	touchInput - Reads touch sensors and returns number of active touch targets.
 *               Also saves number of active touch targets to Tactive and sets
 *               Ttarget to sequentially highest tripped touch target (1-4).
 *               If Tactive == 1, then Ttarget indicates the active target.
 *               If Tactive > 1, compare Tdelta[]s to determine active target.
 *============================================================================*/

unsigned char touchInput(void)
{
    Tactive = 0;			// Reset touch counter
    for(unsigned char i = 0; i != 4; i++)	// Check touch pads for new touch
    {
        CPSCON1 = i;		// Select each of the 4 touch sensors in turn
        TMR0 = 0;			// Clear cap oscillator cycle timer
        __delay_us(1000);   // Wait for fixed sensing time-base
        Tcount[i] = TMR0;	// Save current oscillator cycle count
        Tdelta[i] = (Tavg[i] - Tcount[i]);	// Calculate touch delta
        Ttrip[i] = Tavg[i] / 8; // Set trip point -12.5% below average
        if(Tcount[i] < (Tavg[i] - Ttrip[i]))    // Tripped?
        {
            Tactive ++;		// Increment active count for tripped sensors
            Ttarget[i] = 1; // Save current touch target as real number
        }
        else                // Not tripped?
        {
            Ttarget[i] = 0;
            if(Tcount[i] > Tavg[i]) // Average < count?
            {
                Tavg[i] = Tcount[i];    // Set average to prevent underflow
            }
            else            // Or, calculate new average
            {
                Tavg[i] = Tavg[i] - (Tavg[i] / 16) + (Tcount[i] / 16);                    
            }
        }
    }
    return(Tactive);
}

/*==============================================================================
 *	Main program loop. The main() function is called first by the C compiler.
 *============================================================================*/

int main(void)
{
	init();						// Initialize oscillator, I/O, and peripherals
	initTouch();				// Calibrate capacitive touch sensor averages
	
//    TMR2ON = 1;
    
	// Repeat the main loop continuously
	
	while(1)
	{
		SWDTEN = 0;				// Disable Watch Dog Timer
		
        while(mode == off)
        {
            CPSON = 0;              // Disable CapSense module
            SWDTEN = 1;				// Enable Watch Dog Timer
            SLEEP();				// Nap to save power. Wake up in ~32 ms.

            SWDTEN = 0;             // Disable WDT
            if(S1 == 0 && modeSwitch == 0)  // Check for button press after wake-up
            {
                SWDTEN = 0;         // Disable Watch Dog
                CPSON = 1;          // Enable CapSense module
                modeSwitch = true;
                mode = piano;       // Switch to piano mode
            }
            
            if(S1 == 1)
            {
                modeSwitch = false;
            }
        }

        while(mode == piano)
        {
            if(touchInput() > 0)   // Check for touch sensor activity
            {
                if(Ttarget[0] == 1 && Ttarget[1] == 0)  // Right-most key
                {
                    note = 7;
                }
                else if(Ttarget[0] == 1 && Ttarget[1] == 1)
                {
                    note = 6;
                }
                else if(Ttarget[1] == 1 && Ttarget[2] == 0)
                {
                    note = 5;
                }
                else if(Ttarget[1] == 1 && Ttarget[2] == 1)
                {
                    note = 4;
                }
                else if(Ttarget[2] == 1 && Ttarget[3] == 0)
                {
                    note = 3;
                }
                else if(Ttarget[2] == 1 && Ttarget[3] == 1)
                {
                    note = 2;
                }
                else if(Ttarget[3] == 1)    // Left-most key
                {
                    note = 1;
                }
            }
            else
            {
                note = 0;
            }
		
            if(S1 == 0 && modeSwitch == 0)  // Check for mode switch
            {
                modeSwitch = true;
                mode = metronome;
                beatOn = true;
            }
            
            if(S1 == 1)                 // Reset mode switch activity
            {
                modeSwitch = false;
            }

            if(note == 8)               // A5
            {
                TMR2ON = 1;             // Make notes using PWM module
                PR2 = 68;               // PWM period
                CCPR1L = 34;            // PWM value
            }
            else if(note == 7)          // G#5
            {
                TMR2ON = 1;
                PR2 = 72;
                CCPR1L = 36;
            }
            else if(note == 6)          // F#5
            {
                TMR2ON = 1;
                PR2 = 81;
                CCPR1L = 40;
            }
            else if(note == 5)          // E5
            {
                TMR2ON = 1;
                PR2 = 91;
                CCPR1L = 45;
            }
            else if(note == 4)          // D5
            {
                TMR2ON = 1;
                PR2 = 102;
                CCPR1L = 51;
            }
            else if(note == 3)          // C#5
            {
                TMR2ON = 1;
                PR2 = 108;
                CCPR1L = 54;
            }
            else if(note == 2)          // B4
            {
                TMR2ON = 1;
                PR2 = 121;
                CCPR1L = 61;
            }
            else if(note == 1)          // A4
            {
                TMR2ON = 1;
                PR2 = 136;
                CCPR1L = 68;
            }
            else
            {
                TMR2ON = 0;
            }
        }
        
        while(mode == metronome)
        {
            if(beatOn == true)              // Make beats if metronome is running
            {
                bpmIndex = bpm - 40;        // Convert from bpm to delay using
                if(bpmIndex == 0)           // table look-up
                {
                    makeBeat(beatDelay[0] - 20);
                }
                else
                {
                    bpmIndex = bpmIndex / 5;
                    makeBeat(beatDelay[bpmIndex] - 20);
                }
            }
            
            if(S1 == 0 && modeSwitch == 0)  // Check for mode switch
            {
                modeSwitch = true;
                mode = off;
            }
            
            if(S1 == 1)                     // Reset mode switch activity
            {
                modeSwitch = false;
            }
            
            if(touchInput() > 0)
            {
                if(Ttarget[0] == 1 && settingChange == false)   // Beat/measure
                {
                    settingChange = true;
                    beats++;
                    if(beats >= 9)
                    {
                        beats = 1;
                        beat = 0;
                    }
                }
                else if(Ttarget[1] == 1)        // Increase bpm in steps of 5
                {
                    if(bpm < 240)
                    {
                        bpm += 5;
                    }
                }
                else if(Ttarget[2] == 1)        // Decrease bpm in steps of 5
                {
                    if(bpm > 60)
                    {
                        bpm -= 5;
                    }
                }
                else if(Ttarget[3] == 1 && settingChange == false)
                {
                    settingChange = true;
                    beatOn = !beatOn;           // Toggle beats
                }
            }
            else
            {
                settingChange = 0;
            }
        }
        
	}
}
