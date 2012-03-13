//******************************************************************************
// saa1064 replacement
//  ***THIS IS THE SLAVE CODE***
//
//                  Slave                      Master
//                                      (msp430x20x3_usi_06.c)
//               MSP430F20x2/3              MSP430F20x2/3
//             -----------------          -----------------
//         /|\|              XIN|-    /|\|              XIN|-
//          | |                 |      | |                 |
//          --|RST          XOUT|-     --|RST          XOUT|-
//            |                 |        |                 |
//      LED <-|P1.0             |        |                 |
//            |                 |        |             P1.0|-> LED
//            |         SDA/P1.7|------->|P1.7/SDA         |
//            |         SCL/P1.6|<-------|P1.6/SCL         |
//
//******************************************************************************

#define __MSP430G2252__ 1
//#define __MSP430G2231__ 1
#include <msp430.h>

//#include <msp430g2231.h>
//#include <msp430g2452.h>

//#include <math.h>
//#include <isr_compat.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

//#include  <msp430x20x2.h>

char MST_Data = 0;                     // Variable for received data
char SLV_Addr = 0x90;                  // Address is 0x48<<1 for R/W
int I2C_State = 0;                     // State variable

unsigned char status = 0x01, dir = 0x01;
unsigned char p = 0x80;

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
	Setup_USI_Slave();

	WDTCTL = WDTPW | WDTHOLD; /* Watchdog Timer Control = */

// 	P1IFG = BIT3; //                0x0023    Port 1 Interrupt Flag
 	P1IES |= BIT3; //                0x0024    Port 1 Interrupt Edge Select
 	P1IE |= BIT3; //                 0x0025    Port 1 Interrupt Enable

	P1DIR |= (BIT0 | BIT6);
	P1OUT = BIT0;

	TACTL |= MC_1;
	TACTL |= TASSEL_1;
	TACTL |= TAIE;
//	TACTL |= TACLR;

	BCSCTL3 |= LFXT1S_2;

	TACCTL0 = CCIE;
//	CCR0 = TAR; // Current state of TA counter
//	CCR0 = 0x7fff;
//	CCTL0 = CCIE; // enable capture/compare int 
//	P1DIR |= 0x01; // Set P1.0 to output direction


	TACCR0 = p; //0x0fff;  //SMCLK/TIME_1MS;
//	TAR = 0;
//	TA0CTL |= (TASSEL_1 | MC_1 | ID_0 | TACLR | TAIE); // SMCLK

	eint();

	for (;;)
	{
		nop();
		WRITE_SR(GIE | CPUOFF);
	}
}

//interrupt void port1_isr(void)
//{
//	status ^= 0x0001;
//};
//ISR(PORT1,port1_isr)

//interrupt(PORT1_VECTOR) port1_isr(void)

ISR(PORT1,port1_isr)
{
 	P1IFG &= ~BIT3; //                0x0023    Port 1 Interrupt Flag
	if(p == 0xfe)
		dir = 0x0;
	if(p == 0x02)
		dir = 0x1;

	if(dir == 0x01)
		p += 0x2;
	else
		p -= 0x2;
};

ISR(TIMER0_A0,timer0_a3_isr)
{
	TACCR0 = 0xff & ((0x01 & (status++)) ? p : ~p );
	P1OUT ^= BIT0 | BIT6;
 	TACTL &= ~TAIFG;
};
