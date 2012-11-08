#define __MSP430G2252__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

#include "led.h"

//digit_t digits[4];
unsigned char* digits = 0;
unsigned char* config_p = 0;

unsigned char status = 0x01, dir = 0x01, p = 0x80, d = 0;

void Init_display(void) {

//	digits[0].byte = SEG_ZERO;
//	digits[1].byte = SEG_ONE;
//	digits[2].byte = SEG_TWO;
//	digits[3].byte = SEG_THREE;

//	((digit_t*)(&digits[0]))->byte = ~SEG_BLANK;
//	((digit_t*)(&digits[1]))->byte = ~SEG_BLANK;
//	((digit_t*)(&digits[2]))->byte = ~SEG_BLANK;
//	((digit_t*)(&digits[3]))->byte = ~SEG_BLANK;

	((digit_t*)(&digits[0]))->byte = SEG_BLANK;
	((digit_t*)(&digits[1]))->byte = SEG_BLANK;
	((digit_t*)(&digits[2]))->byte = SEG_BLANK;
	((digit_t*)(&digits[3]))->byte = SEG_BLANK;

//	((digit_t*)(&digits[0]))->byte = SEG_ZERO;
//	((digit_t*)(&digits[1]))->byte = SEG_ONE;
//	((digit_t*)(&digits[2]))->byte = SEG_TWO;
//	((digit_t*)(&digits[3]))->byte = SEG_THREE;
}

void Setup_LED(unsigned char* buffer, unsigned char* cfg){

	P1DIR |= (BIT2 | BIT3 | BIT4 | BIT5);
	P2SEL &= ~(BIT6 | BIT7); // changes the function of XIN/XOUT into GPIO
	P2DIR |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

	P1OUT |= (BIT3);
	P2OUT |= (BIT0 | BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

	TACTL |= MC_1;
	TACTL |= TASSEL_1;
	TACTL |= TAIE;

	BCSCTL3 |= LFXT1S_2;

	TACCTL1 = CCIE;
	TACCTL0 = CCIE;

	TACCR1 = 0x20; //0x0fff;  //SMCLK/TIME_1MS;
	TACCR0 = 0x40; //0x0fff;  //SMCLK/TIME_1MS;

	digits = (unsigned char*)buffer;
	config_p = (unsigned char*)cfg;
//	return (unsigned char*)(&digits);
}

interrupt(TIMER0_A1_VECTOR) timer0_a1_isr(void)
{
	if(TACCTL1 && CCIFG) {
		TACCTL1 &= ~CCIFG;
// clear anodes:
		P1OUT &= ~(BIT2 | BIT4 | BIT5);
		P2OUT &= ~(BIT2);

// clear cathodes:
		P1OUT |= (BIT3);
		P2OUT |= (BIT0 | BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
	}
}
// set next digit

interrupt(TIMER0_A0_VECTOR) timer0_a0_isr(void)
{
	TACCR1 = ( (*config_p) & 0x70 ) >> 1;

	++d;
	d &= 0b11;

	switch (d) {
	case 0b00:
		P2OUT |= BIT2;
		break;
	case 0b01:
		P1OUT |= BIT5;
		break;
	case 0b10:
		P1OUT |= BIT4;
		break;
	case 0b11:
		P1OUT |= BIT2;
		break;
	default:
		break;
	}
// output appropriate digit on cathode

	P1OUT &= ~( ((digit_t*)(&digits[d]))->seg_b ? BIT3:0x0 );
	P2OUT &= ~(
		(((digit_t*)(&digits[d]))->seg_a ? BIT1:0x0) |
		(((digit_t*)(&digits[d]))->seg_c ? BIT7:0x0) |
		(((digit_t*)(&digits[d]))->seg_d ? BIT4:0x0) |
		(((digit_t*)(&digits[d]))->seg_e ? BIT3:0x0) |
		(((digit_t*)(&digits[d]))->seg_f ? BIT0:0x0) |
		(((digit_t*)(&digits[d]))->seg_g ? BIT6:0x0) |
		(((digit_t*)(&digits[d]))->seg_h ? BIT5:0x0));

 	TACTL &= ~TAIFG;
};
