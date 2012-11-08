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
#include "adc.h"
#include "i2c_usi.h"

// C0 = 0 static mode, i.e. continuous display of digits 1 and 2
// C0 = 1 dynamic mode, i.e. alternating display of digit 1 + 3 and 2 + 4
#define DISLP_DYNAMIC 0x01

//C1 = 0/1 digits 1 + 3 are blanked/not blanked
#define DISPL_BLANK1 0x02

//C2 = 0/1 digits 2 + 4 are blanked/not blanked
#define DISPL_BLANK2 0x04

//C3 = 1 all segment outputs are switched-on for segment test (1)
#define DISPLAY_TEST 0x08

//C4 = 1 adds 3 mA to segment output current
#define DISPLAY_CURR_3M 0x10

//C5 = 1 adds 6 mA to segment output current
#define DISPLAY_CURR_6M 0x20

//C6 = 1 adds 12 mA to segment output current
#define DISPLAY_CURR_12M 0x40

unsigned char display_buffer[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char adc_buffer[2] = { 0x12, 0x34 };
unsigned char config = DISPLAY_CURR_3M + DISPLAY_CURR_6M + DISPLAY_CURR_12M;

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

	Setup_ADC((unsigned char*)&adc_buffer);
	Setup_LED((unsigned char*)&display_buffer, (unsigned char*)&config);

	Setup_I2C((unsigned char*)&display_buffer, (unsigned char*)&adc_buffer, (unsigned char*)&config);

	Init_display();

	__eint();

	for (;;)
	{
		nop();
		WRITE_SR(GIE | CPUOFF);
	}
}

