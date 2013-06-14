#define __MSP430G2553__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

#include "led8.h"

#define PERIOD 0xFFFF

unsigned char* digits = 0;

unsigned int counter_l = 0;
unsigned int counter_h = 0;
//unsigned int counter_c = 0;

unsigned char status = 0x01, dir = 0x01, p = 0x80, d = 0;

static const unsigned char seven_seg[16] = { SEG_ZERO,
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

void Init_display(void) {

//	digits[0].byte = SEG_ZERO;
//	digits[1].byte = SEG_ONE;
//	digits[2].byte = SEG_TWO;
//	digits[3].byte = SEG_THREE;

#if 0
	((digit_t*)(&digits[0]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[1]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[2]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[3]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[4]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[5]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[6]))->byte = ~SEG_BLANK;
	((digit_t*)(&digits[7]))->byte = ~SEG_BLANK;
#endif
#if 1
	((digit_t*)(&digits[0]))->byte = SEG_BLANK;
	((digit_t*)(&digits[1]))->byte = SEG_BLANK;
	((digit_t*)(&digits[2]))->byte = SEG_BLANK;
	((digit_t*)(&digits[3]))->byte = SEG_BLANK;
	((digit_t*)(&digits[4]))->byte = SEG_BLANK;
	((digit_t*)(&digits[5]))->byte = SEG_BLANK;
	((digit_t*)(&digits[6]))->byte = SEG_BLANK;
	((digit_t*)(&digits[7]))->byte = SEG_BLANK;
#endif
#if 0
	((digit_t*)(&digits[0]))->byte = SEG_ONE;
	((digit_t*)(&digits[1]))->byte = SEG_ONE;
	((digit_t*)(&digits[2]))->byte = SEG_ONE;
	((digit_t*)(&digits[3]))->byte = SEG_ONE;
	((digit_t*)(&digits[4]))->byte = SEG_ONE;
	((digit_t*)(&digits[5]))->byte = SEG_ONE;
	((digit_t*)(&digits[6]))->byte = SEG_ONE;
	((digit_t*)(&digits[7]))->byte = SEG_ONE;
#endif
#if 1
	((digit_t*)(&digits[0]))->byte = SEG_ZERO;
	((digit_t*)(&digits[1]))->byte = SEG_ONE;
	((digit_t*)(&digits[2]))->byte = SEG_TWO;
	((digit_t*)(&digits[3]))->byte = SEG_THREE;
	((digit_t*)(&digits[4]))->byte = SEG_FOUR;
	((digit_t*)(&digits[5]))->byte = SEG_FIVE;
	((digit_t*)(&digits[6]))->byte = SEG_SIX;
	((digit_t*)(&digits[7]))->byte = SEG_SEVEN;
#endif
#if 0
	((digit_t*)(&digits[1]))->byte = SEG_ZERO;
	((digit_t*)(&digits[2]))->byte = SEG_ONE;
	((digit_t*)(&digits[3]))->byte = SEG_TWO;
	((digit_t*)(&digits[4]))->byte = SEG_THREE;
	((digit_t*)(&digits[5]))->byte = SEG_FOUR;
	((digit_t*)(&digits[6]))->byte = SEG_FIVE;
	((digit_t*)(&digits[7]))->byte = SEG_SIX;
	((digit_t*)(&digits[0]))->byte = SEG_SEVEN;
#endif
}

void Setup_LED(unsigned char* buffer) {

	digits = (unsigned char*)buffer;

	P1DIR |= (BIT3 | BIT6 | BIT7);
	P2DIR |= (BIT0 | BIT1 | BIT3 | BIT4 | BIT5);

	BCSCTL3 |= LFXT1S_2;

	TA1CCTL0 = CCIE;
	TA1CCTL1 = CCIE;

	// full period
	TA1CCR0 = PERIOD;
	// duty cycle of CA vs. CC displays
	TA1CCR1 = PERIOD - (PERIOD >> 2);

	// MC_0 - stop
	// MC_1 - up
	// MC_2 - continuous
	// MC_3 - up/down
	// TASSEL_0 - TACLK
	// TASSEL_1 - ACLK
	// TASSEL_2 - SMACLK
	// TASSEL_3 - INCLK
	// ID_0 = /1
	// ID_1 = /2
	// ID_2 = /4
	// ID_3 = /8
	TA1CTL = MC_2 | TASSEL_2 | TAIE;
}

// half way through - we need to switch to CA
interrupt(TIMER1_A1_VECTOR) timer1_a1_isr(void)
{
	if(TA1IV && TA1IV_TACCR1) {

// switch the direction to the common anode digits
		switch (d & 0b11) {
		case 0b00: // 2.2 0
			P2OUT |= BIT2;
			break;
		case 0b01: // 1.5 1
			P1OUT |= BIT5;
			break;
		case 0b10: // 1.4 2
			P1OUT |= BIT4;
			break;
		case 0b11: // 1.2 3
			P1OUT |= BIT2;
			break;
		default:
			break;
		}

		// drive down the cathodes of segments for digits 4...7
		P1OUT |= BIT3 | BIT6 | BIT7;
		P1OUT &= ~( (((digit_t*)(&digits[d]))->seg_b ? BIT3:0x00) |
			    (((digit_t*)(&digits[d]))->seg_c ? BIT6:0x00) |
			    (((digit_t*)(&digits[d]))->seg_g ? BIT7:0x00));
		P2OUT |= BIT0 | BIT1 | BIT3 | BIT4 | BIT5;
		P2OUT &= ~( (((digit_t*)(&digits[d]))->seg_a ? BIT1:0x00) |
			    (((digit_t*)(&digits[d]))->seg_d ? BIT4:0x00) |
			    (((digit_t*)(&digits[d]))->seg_e ? BIT3:0x00) |
			    (((digit_t*)(&digits[d]))->seg_f ? BIT0:0x00) |
			    (((digit_t*)(&digits[d]))->seg_h ? BIT5:0x00) );

//		TA1CTL &= ~TAIFG;
	}
}

// begin of display period
interrupt(TIMER1_A0_VECTOR) timer1_a0_isr(void)
{
	++d;
	d &= 0b11;

	TA1CTL &= ~TAIFG;

//	if(TA1CCTL1 && CCIFG) {
// pull down selected digit with digit driver down
		switch (d) {
		case 0b00: // 2.2 4
			P1DIR &= ~BIT2;
			P2OUT &= ~BIT2;
			P2DIR |= BIT2;
			break;
		case 0b01: // 1.5 5
			P2DIR &= ~BIT2;
			P1OUT &= ~BIT5;
			P1DIR |= BIT5;
			break;
		case 0b10: // 1.4 6
			P1DIR &= ~BIT5;
			P1OUT &= ~BIT4;
			P1DIR |= BIT4;
			break;
		case 0b11: // 1.2 7
			P1DIR &= ~BIT4;
			P1OUT &= ~BIT2;
			P1DIR |= BIT2;
			break;
		default:
			break;
		}
// output appropriate digit on segment drivers

		P1OUT &= ~(BIT3 | BIT6 | BIT7);
		P1OUT |= ( (((digit_t*)(&digits[d+4]))->seg_b ? BIT3:0x00) |
			   (((digit_t*)(&digits[d+4]))->seg_c ? BIT6:0x00) |
			   (((digit_t*)(&digits[d+4]))->seg_g ? BIT7:0x00) );
		P2OUT &= ~(BIT0 | BIT1 | BIT3 | BIT4 | BIT5);
		P2OUT |= ( (((digit_t*)(&digits[d+4]))->seg_a ? BIT1:0x00) |
			   (((digit_t*)(&digits[d+4]))->seg_d ? BIT4:0x00) |
			   (((digit_t*)(&digits[d+4]))->seg_e ? BIT3:0x00) |
			   (((digit_t*)(&digits[d+4]))->seg_f ? BIT0:0x00) |
			   (((digit_t*)(&digits[d+4]))->seg_h ? BIT5:0x00) );

//		P1DIR |= (BIT3 | BIT6 | BIT7);
//		P2DIR |= (BIT0 | BIT1 | BIT3 | BIT4 | BIT5);

// set next digit
#if 1
		counter_l += 0x100;
		if(!counter_l) {
			++counter_h;
			counter_h &= 0xf;
			((digit_t*)(&digits[0]))->byte = ((digit_t*)(&digits[1]))->byte;
			((digit_t*)(&digits[1]))->byte = ((digit_t*)(&digits[2]))->byte;
			((digit_t*)(&digits[2]))->byte = ((digit_t*)(&digits[3]))->byte;
			((digit_t*)(&digits[3]))->byte = ((digit_t*)(&digits[4]))->byte;
			((digit_t*)(&digits[4]))->byte = ((digit_t*)(&digits[5]))->byte;
			((digit_t*)(&digits[5]))->byte = ((digit_t*)(&digits[6]))->byte;
			((digit_t*)(&digits[6]))->byte = ((digit_t*)(&digits[7]))->byte;
			((digit_t*)(&digits[7]))->byte = seven_seg[ counter_h ];
		}
#endif //0
#if 0
//	++counter_c;
//	counter_c &= 0x3;

//	if(!counter_c) {
//		++counter_l;
		counter_l += 256;

		if(!counter_l) {
			++counter_h;

			if(counter_h & 0xf000)
				((digit_t*)(&digits[0]))->byte = seven_seg[ (counter_h >> 12) & 0xf ];
			else
				((digit_t*)(&digits[0]))->byte = SEG_BLANK;
			if(counter_h & 0xff00)
				((digit_t*)(&digits[1]))->byte = seven_seg[ (counter_h >> 8) & 0xf ];
			else
				((digit_t*)(&digits[1]))->byte = SEG_BLANK;
			if(counter_h & 0xfff0)
				((digit_t*)(&digits[2]))->byte = seven_seg[ (counter_h >> 4) & 0xf ];
			else
				((digit_t*)(&digits[2]))->byte = SEG_BLANK;
			if(counter_h)
				((digit_t*)(&digits[3]))->byte = seven_seg[ counter_h & 0xf ];
			else
				((digit_t*)(&digits[3]))->byte = SEG_BLANK;
		}
		if(counter_h || (counter_l & 0xf000))
			((digit_t*)(&digits[4]))->byte = seven_seg[ (counter_l >> 12) & 0xf ];
		else
			((digit_t*)(&digits[4]))->byte = SEG_BLANK;
		if(counter_h || (counter_l & 0xff00))
			((digit_t*)(&digits[5]))->byte = seven_seg[ (counter_l >> 8) & 0xf ];
		else
			((digit_t*)(&digits[5]))->byte = SEG_BLANK;
		if(counter_h || (counter_l & 0xfff0))
			((digit_t*)(&digits[6]))->byte = seven_seg[ (counter_l >> 4) & 0xf ];
		else
			((digit_t*)(&digits[6]))->byte = SEG_BLANK;
		((digit_t*)(&digits[7]))->byte = seven_seg[ counter_l & 0xf ];
//	}
#endif
//		TA1CTL &= ~TAIFG;
//	}
};
