//******************************************************************************
//
//                  Slave                      Master
//               MSP430G2253
//             -----------------          -----------------
//            |                 |        |                 |
//            |         SDA/P1.7|------->|SDA              |
//            |         SCL/P1.6|<-------|SCL              |
//            |                 |        |                 |
//            |                 |         -----------------
//            |                 |
//            |                 |         -----------------
//            |                 |        |                 |
//            |             P2.1|------->|SEG A            |
//            |             P1.3|------->|SEG B            |
//            |             P2.7|------->|SEG C            |
//            |             P2.4|------->|SEG D            |
//            |             P2.3|------->|SEG E            |
//            |             P2.0|------->|SEG F            |
//            |             P2.6|------->|SEG G            |
//            |             P2.5|------->|SEG H            |
//            |                 |        |                 |
//            |             P2.2|------->|COM 0            |
//            |             P1.5|------->|COM 1            |
//            |             P1.4|------->|COM 2            |
//            |             P1.2|------->|COM 3            |
//            |                 |        |                 |
//             -----------------          -----------------
//
//******************************************************************************

#define __MSP430G2252__ 1
//#define __MSP430G2231__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
//#include <legacymsp430.h>

#include <isr_compat.h>

#include "led.h"
#include "i2c_usi.h"

digit_t* display_buffer = 0;

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog
	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)
	{  
		while(1);                               // If calibration constants erased
		// do not load, trap CPU!!
	}

	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;

	display_buffer = Setup_LED();
	Setup_I2C((unsigned char*)display_buffer);

	Init_display();

	__eint();

	for (;;)
	{
		nop();
		WRITE_SR(GIE | CPUOFF);
	}
}

