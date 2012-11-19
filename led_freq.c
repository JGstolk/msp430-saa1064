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

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog
	if (CALBC1_16MHZ == 0xFF || CALDCO_16MHZ == 0xFF)
	{  
		while(1);                               // If calibration constants erased
		// do not load, trap CPU!!
	}

#if 0
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;
#else
	DCOCTL = 0;
	BCSCTL1 = CALBC1_16MHZ;                    // Set DCO
	DCOCTL = CALDCO_16MHZ;
#endif

	Setup_LED((unsigned char*)&display_buffer);

	Init_display();

	__eint();

	for (;;)
	{
		nop();
//		WRITE_SR(GIE);
		WRITE_SR(GIE | CPUOFF);
	}
}

