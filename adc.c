#define __MSP430G2252__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

#include "adc.h"

unsigned char* adc = 0;

/**
* Reads ADC 'chan' once using an internal reference, 'ref' determines if the
*   2.5V or 1.5V reference is used.
**/
void Single_Measure_Temp()
{
	ADC10CTL0 &= ~ENC;							// Disable ADC
//	ADC10CTL0 = SREF0 + ADC10SHT_2 + ADC10ON + REFON + REF2_5V;
//	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ref + ADC10IE;	// Use reference,
//	ADC10CTL0 = SREF0 + ADC10SHT_2 + ADC10ON + REFON + REF2_5V + ADC10IE;	// Use reference,
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + ADC10ON + REFON + ADC10IE;	// Use reference,
										//   16 clock ticks, internal reference on
										//   ADC On, enable ADC interrupt, Internal  = 'ref'
	ADC10CTL1 = ADC10SSEL_3 + INCH_10;				// Set 'chan', SMCLK
	__delay_cycles (128);					// Delay to allow Ref to settle
	ADC10CTL0 |= ENC + ADC10SC; 				// Enable and start conversion
}

/**
* ADC interrupt routine. Pulls CPU out of sleep mode for the main loop.
**/
interrupt(ADC10_VECTOR) adc10_isr(void)
{

//	adc[0] = 0x56;
//	adc[1] = 0x78;

	adc[0] = 0xff & ADC10MEM;	// Saves measured value.
	adc[1] = 0xff & (ADC10MEM >> 8);

//	Single_Measure_REF(INCH_11, REF2_5V);
	Single_Measure_Temp();
}

void Setup_ADC(unsigned char* buffer){

	P1DIR |= (BIT2 | BIT3 | BIT4 | BIT5);
	P2SEL &= ~(BIT6 | BIT7); // changes the function of XIN/XOUT into GPIO
	P2DIR |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

	P1OUT |= (BIT3);
	P2OUT |= (BIT0 | BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

	TACTL |= MC_1;
	TACTL |= TASSEL_1;
	TACTL |= TAIE;

	BCSCTL3 |= LFXT1S_2;

	adc = (unsigned char*)buffer;

//	Single_Measure_REF(INCH_11, REF2_5V);
	Single_Measure_Temp();
}

