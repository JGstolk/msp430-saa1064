//******************************************************************************
//
//               MSP430G2553
//             -----------------          -----------------
//            |                 |        |                 |
//            |             P1.0|<-------|measured signal  |
//            |                 |        |                 |
//            |                 |         -----------------
//            |                 |
//            |                 |         -----------------
//            |                 |        |                 |
//            |             P2.1|------->|SEG A            |
//            |             P1.3|------->|SEG B            |
//            |             P1.6|------->|SEG C            |
//            |             P2.4|------->|SEG D            |
//            |             P2.3|------->|SEG E            |
//            |             P2.0|------->|SEG F            |
//            |             P1.7|------->|SEG G            |
//            |             P2.5|------->|SEG H            |
//            |                 |        |                 |
//            |             P2.2|------->|COM 0            |
//            |             P1.5|------->|COM 1            |
//            |             P1.4|------->|COM 2            |
//            |             P1.2|------->|COM 3            |
//            |                 |        |                 |
//             -----------------          -----------------
//
//
//		digit driver CA/CC
//		|\
//		| \
//		|  \
//		|   \________________________
//		|   /             |          |
//		|  /              |          |
//		| /               |          |
//		|/                |          |
//                               ___        ___
//                               \ /         ^
//		segment driver   _V_        /_\
//		|\                |          |
//		| \               | CA       | CC
//		|  \              |          |
//		|   \_____________|__________|
//		|   /
//		|  /
//		| /
//		|/
//
//		selecting a digit is caused by setting digit driver to drive an output
//		- switching from input mode to output
//		- driving low for CA half
//		- driving high tfor CC half
//		- driving high the segment to iluminate cA
//		- driving log for the segment to iluminate CC
//
//******************************************************************************

#define __MSP430G2553__ 1
//#define __MSP430G2231__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
//#include <legacymsp430.h>

#include <isr_compat.h>

#include "led8.h"

  //unsigned char display_buffer[8] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
unsigned char display_buffer[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  //unsigned char display_buffer[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

static const unsigned char seven_seg[16] = {
	SEG_ZERO,
	SEG_ONE,
	SEG_TWO,
	SEG_THREE,
	SEG_FOUR,
	SEG_FIVE,
	SEG_SIX,
	SEG_SEVEN,
	SEG_EIGHT,
	SEG_NINE,
	SEG_AA,
	SEG_BB,
	SEG_CC,
	SEG_DD,
	SEG_EE,
	SEG_FF };

// shamelessly copied from oPPossum's nokia5110/lcd/frequency meter
#if 1
static void print_freq(unsigned long n)
{
	int x = 7, dt = 0;

	unsigned long t;

	if(n == 0)
		display_buffer[x--] = SEG_ZERO;
	else
		while((n != 0) && (x >= 0)) {
			t = n / 10;
			dt = n - (t * 10);
			if((dt != 0) || (t != 0))
				display_buffer[x] = seven_seg[dt];
			else
				display_buffer[x] = 0x0;
//		display_buffer[x--] = seven_seg[n % 10];
			n = t;
//		n /= 10;
			--x;
		}
	while(x >= 0)
                display_buffer[x--] = 0x0;
}
#else
static const unsigned long dv[] = {                      // Base 10 digit weights
          10000000,                                                              // 8 digit maximum count
           1000000,                                                              //
                100000,                                                          //
                 10000,                                                          //
                  1000,                                                          //
                   100,                                                          //
                        10,                                                              //
                         1,                                                              //
                         0                                                                //
};

static void print_freq(unsigned long n)
{
        const unsigned long *dp = dv;
        unsigned x = 0;
        unsigned c;
        unsigned long d;
        
        while(n < *dp) {                                                        // Skip leading zeros
//                if(*dp == 100000 || *dp == 100) x += 2; // Space between 3 digit groups
                ++dp;                                                              //
//                lcd_pd10(10, x, 2);                                      // Print space
                display_buffer[x] = 0x0;                                      // Print space
                ++x;                                                                //
        }                                                                                  //
        if(n) {                                                                  // Non-zero
                do {                                                                    //
                        d = *dp++;                                                // Get digit value
                        c = 0;                                                    //
                        while(n >= d) ++c, n -= d;                // Divide
//                        if(d == 100000 || d == 100) x += 2; // Space between 3 digit groups
//                        lcd_pd10(c, x, 2);                                // Print digit
                        display_buffer[x] = seven_seg[c];                                // Print digit
                        ++x;                                                        //
                } while(!(d & 1));                                        // Until all digits done
        } else                                                                    //
                display_buffer[x] = SEG_ZERO;                          // Print zero
//                lcd_pd10(0, x - 10, 2);                          // Print zero
}
#endif

void set_gate(unsigned long f)
{
        if(WDTCTL & WDTIS0) {                                      // 250 ms gate currently in use
                if(f < 800000) {                                                // Switch to 1 s gate if frequncy is below 800 kHz
//                        lcd_print("1 Second Gate", 3, 5);
                        WDTCTL = WDTPW | WDTTMSEL | WDTSSEL;
                }
        } else {                                                                        // 1 s gate currently in use
                if(f > 900000) {                                                // Switch to 250 ms gate if frequency above 900 kHz
//                        lcd_print(" 250 ms Gate ", 3, 5);
                        WDTCTL = WDTPW | WDTTMSEL | WDTSSEL | WDTIS0;
                }
        }
}

static unsigned clock_input = 1;

//#define TASSEL_0            (0x0000) /* Timer A clock source select: 0 - TACLK */
//#define TASSEL_1            (0x0100) /* Timer A clock source select: 1 - ACLK  */
//#define TASSEL_2            (0x0200) /* Timer A clock source select: 2 - SMCLK */
//#define TASSEL_3            (0x0300) /* Timer A clock source select: 3 - INCLK */

void set_input(void)
{
        const unsigned char z = 3;
//        lcd_pos(0, 4);
//        lcd_send(&z, 84, lcd_data_repeat);
        
        switch(clock_input) {
	default:
		clock_input = 0;
	case 0:
		TACTL = TASSEL_2;
//		lcd_print("Internal 16MHz", 0, 4);
		break;
	case 1:
		TACTL = TASSEL_0;
//		lcd_print("Clock In P1.0", 3, 4);
		break;
#if 1
	case 2:                                                          // This should always show 32768       
		TACTL = TASSEL_1;                                  //  Something is very wrong if it doesn't
//		lcd_print("Internal 32kHz", 0, 4);
		break;
#endif            
#if 1
	case 3:
		TACTL = TASSEL_3; // INCLK
		break;
#endif
        }
}

int main(void)
{
        unsigned long freq = 12345678L;                  // Measured frequency

	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog
	if (CALBC1_16MHZ == 0xFF || CALDCO_16MHZ == 0xFF)
	{  
		while(1);                               // If calibration constants erased
		// do not load, trap CPU!!
	}

	DCOCTL = 0;
	BCSCTL1 = CALBC1_16MHZ;                    // Set DCO
	DCOCTL = CALDCO_16MHZ;

	Setup_LED((unsigned char*)&display_buffer);

	Init_display();

        P1SEL |= BIT0;                                                    // Use P1.0 as TimerA input
        P1SEL2 &= ~BIT0;                                                        //
        P1DIR &= ~BIT0;                                                  // 
        P1OUT &= ~BIT0;                                                  // Enable pull down resistor to reduce stray counts
        P1REN |= BIT0;                                                    //

	__eint();

        WDTCTL = WDTPW | WDTTMSEL | WDTCNTCL | WDTSSEL | WDTIS0; // Use WDT as interval timer
                                                   // Default to 250 ms gate so that initial call to set_gate()
                                                   //  will switch to 1 s gate and update the LCD
        set_input();                               // Set input and show on LCD

	//lcd_print("1 Second Gate", 3, 5);
//	WDTCTL = WDTPW | WDTTMSEL | WDTSSEL;
	//lcd_print(" 250 ms Gate ", 3, 5);
//	WDTCTL = WDTPW | WDTTMSEL | WDTSSEL | WDTIS0;
 
	for (;;)
	{
                freq = 0;                           // Clear frequency
                TACTL |= TACLR;                     // Clear TimerA
                                                    //
                IFG1 &= ~WDTIFG;                    // Wait for WDT period to begin
                while(!(IFG1 & WDTIFG));            //
                                                    //
                TACTL |= MC_2;                      // Start counting - TimerA continuous mode
                                                    //
                IFG1 &= ~WDTIFG;                    //
                while(!(IFG1 & WDTIFG)) {           // While WDT period..
                        if(TACTL & TAIFG) {         // Check for TimerA overflow
                                freq += 0x10000L;   // Add 1 to msw of frequency
                                TACTL &= ~TAIFG;    // Clear overflow flag
                        }                           //
                }                                   //
                                                    //
                TACTL &= ~MC_2;                     // Stop counting - TimerA stop mode
                if(TACTL & TAIFG) freq += 0x10000L; // Handle TimerA overflow that may have occured between
                                                    //  last check of overflow and stopping TimerA 
                freq |= TAR;                        // Merge TimerA count with overflow 
                if(WDTCTL & WDTIS0) freq <<= 2;     // Multiply by 4 if using 250 ms gate
                print_freq(freq);                   // Show on LCD
                                                    //
                set_gate(freq);                     // Adjust gate time if necessary
                                                    //
//                if(!(P1IN & BIT3)) {              // Check if pushbutton down
//                        ++clock_input;            // Switch clock input
//                        set_input();              //
//                }                                 //
	}
}

