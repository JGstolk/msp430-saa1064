
#define __MSP430G2252__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

#include "i2c_usi.h"
#include "led.h"

//unsigned char MST_Data[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char* MST_Data = 0;
unsigned char SLV_Data = 0x55;

unsigned int I2C_State, Bytecount;     // State variables

void Data_RX(char c){

//	MST_Data[Bytecount] = c;
//	MST_Data[Bytecount] = Bytecount;

	USICTL0 &= ~USIOE;            // SDA = input
	USICNT |=  0x08;              // Bit counter = 8, RX data
	I2C_State = 8;                // next state: Test data and (N)Ack
}

void TX_Data(void){
	USICTL0 |= USIOE;             // SDA = output
	USISRL = SLV_Data++;
	USICNT |=  0x08;              // Bit counter = 8, TX data
	I2C_State = 12;               // Go to next state: receive (N)Ack
}

//******************************************************************************
// USI interrupt service routine
// Rx bytes from master: State 2->4->6->8 
// Tx bytes to Master: State 2->4->10->12->14
//******************************************************************************
interrupt(USI_VECTOR) usi_i2c_txrx(void)
{
	if (USICTL1 & USISTTIFG)                  // Start entry?
		I2C_State = 2;                          // Enter 1st state on start

	switch(I2C_State) {

	case 2: // RX Address
		USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, RX address
		USICTL1 &= ~USISTTIFG;        // Clear start flag
		I2C_State = 4;                // Go to next state: check address
		break;

	case 4: // Process Address and send (N)Ack

		if ( (USISRL & 0xFE) == I2C_Addr)       // Address match?
		{
			USICTL0 |= USIOE;             // SDA = output
			USISRL = 0x00;              // Send Ack
			I2C_State = (USISRL & 0x01) ? 10 : 6;
			USICNT |= 0x01;               // Bit counter = 1, send (N)Ack bit
			Bytecount = 0;                // Reset counter for next TX/RX
		} else
			I2C_State = 2;              // next state: prep for next Start
		break;

	case 6: // Receive data byte

//		MST_Data[0] = SEG_SIX;

		Data_RX(USISRL);
		break;

	case 8:// Check Data & TX (N)Ack
		if (Bytecount <= 4 ) { // expected number of bytes // If not last byte
			MST_Data[Bytecount] = USISRL;
			Bytecount++;
			USICTL0 |= USIOE;             // SDA = output
			USISRL = 0x00;              // Send Ack
			I2C_State = 6;              // Rcv another byte
			USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
		} else {                          // Last Byte
			USICTL0 |= USIOE;             // SDA = output
			USISRL = 0xFF;              // Send NAck
			USICTL0 &= ~USIOE;            // SDA = input
			I2C_State = 0;                // Reset state machine
			Bytecount = 0;                // Reset counter for next TX/RX
		}
		break;

	case 10: // Send Data byte
		TX_Data();
		break;

	case 12:// Receive Data (N)Ack
		USICTL0 &= ~USIOE;            // SDA = input
		USICNT |= 0x01;               // Bit counter = 1, receive (N)Ack
		I2C_State = 14;               // Go to next state: check (N)Ack
		break;

	case 14:// Process Data Ack/NAck
		if (USISRL & 0x01)               // If Nack received...
		{
			USICTL0 &= ~USIOE;            // SDA = input
			I2C_State = 0;                // Reset state machine
			Bytecount = 0;
			// LPM0_EXIT;                  // Exit active for next transfer
		} else                          // Ack received
			TX_Data();                  // TX next byte
		break;
	default:
	case 0:                               // Idle, should not get here
		break;
	}
	USICTL1 &= ~USIIFG;                       // Clear pending flags
}

void Setup_I2C(unsigned char* buff){

	P1DIR |= (BIT6 | BIT7);
	P1OUT |= (BIT6 | BIT7);             // P1.6 & P1.7 Pullups / P1.3 segment C
	P1REN |= (BIT6 | BIT7);             // P1.6 & P1.7 Pullups

	USICTL0 = USIPE6+USIPE7+USISWRST;         // Port & USI mode setup
	USICTL1 = USII2C+USIIE+USISTTIE;          // Enable I2C mode & USI interrupts
	USICKCTL = USICKPL;                       // Setup clock polarity
	USICNT |= USIIFGCC;                       // Disable automatic clear control
	USICTL0 &= ~USISWRST;                     // Enable USI
	USICTL1 &= ~USIIFG;                       // Clear pending flag
  
	MST_Data = buff;
}
