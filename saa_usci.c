//******************************************************************************
//
//                  Slave                      Master
//               MSP430G2253
//             -----------------          -----------------
//            |                 |        |                 |
//            |         SDA/P1.7|------->|P1.7/SDA         |
//            |         SCL/P1.6|<-------|P1.6/SCL         |
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

//#define __MSP430G2453__ 1
//#include <msp430.h>

//#include <msp430g2453.h>

#include "msp430x26x.h"

//depreciated: #include <signal.h>
  //#include <legacymsp430.h>

#include <isr_compat.h>

#include "TI_USCI_I2C_slave.h"
#include "led.h"

unsigned char TXData[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char RXData[4] = { 0x00, 0x00, 0x00, 0x00 };

unsigned char flag = 0x00;
unsigned char flag1 = 0x00;

void start_cb(void);
void transmit_cb(unsigned char volatile *receive);
void receive_cb(unsigned char receive);

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;                            // Stop WDT
  
	TI_USCI_I2C_slaveinit((void (*)(volatile unsigned char *))start_cb,
			      (void (*)(volatile unsigned char *))transmit_cb,
			      (void (*)(volatile unsigned char))receive_cb,
			      0x50); // init the slave

	Setup_LED();

	Init_display();

	__eint();
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;
	LPM0;                                                // Enter LPM0.
}

void start_cb(void){
	flag = 0;
	flag1 = 0;
}

void receive_cb(unsigned char receive){
	flag1 &= 0b11;
	RXData[flag1++] = receive;
}

void transmit_cb(unsigned char volatile *byte){
	flag1 &= 0b11;
	*byte = TXData[flag++];
}
