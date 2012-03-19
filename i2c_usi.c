
#define __MSP430G2252__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

#include "i2c_usi.h"

//unsigned char MST_Data[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char* MST_Data = 0;
char* MST_Data_p = 0;                          // Variable for received data
unsigned char SLV_Data = 0x55;

// make sure it is even address:
#define I2C_Addr 0x70
// Address is 0x38<<1 for R/W

unsigned int I2C_State, Bytecount, transmit = 0;     // State variables

void Data_RX(void){
//	MST_Data[MST_Data_cnt++] = USISRL;
	*MST_Data_p = USISRL;
	MST_Data_p++;
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

//	switch(__even_in_range(I2C_State,14))
	switch(I2C_State) {

	case 2: // RX Address
		USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, RX address
		USICTL1 &= ~USISTTIFG;        // Clear start flag
		I2C_State = 4;                // Go to next state: check address
		break;

	case 4: // Process Address and send (N)Ack

		transmit = (USISRL & 0x01);            // If master read...

		USICTL0 |= USIOE;             // SDA = output
		if ( (USISRL & 0xFE) == I2C_Addr)       // Address match?
		{
			USISRL = 0x00;              // Send Ack
			I2C_State = transmit ? 10 : 6;
//			if (transmit == 0) 
//				I2C_State = 6;           // Go to next state: RX data
//			if (transmit == 1)  
//				I2C_State = 10;          // Else go to next state: TX data
		} else {
			USISRL = 0xFF;              // Send NAck
			I2C_State = 8;              // next state: prep for next Start
		}
		USICNT |= 0x01;               // Bit counter = 1, send (N)Ack bit
		break;

	case 6: // Receive data byte
		Data_RX();
		break;  

	case 8:// Check Data & TX (N)Ack
		USICTL0 |= USIOE;             // SDA = output
		if (Bytecount <= 2)
			// If not last byte
		{
			USISRL = 0x00;              // Send Ack
			I2C_State = 6;              // Rcv another byte
			Bytecount++;
			USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
		} else {                          // Last Byte
			USISRL = 0xFF;              // Send NAck
			USICTL0 &= ~USIOE;            // SDA = input
//			SLV_Addr = I2C_Addr;              // Reset slave address
			I2C_State = 0;                // Reset state machine
			Bytecount = 0;                // Reset counter for next TX/RX
//			MST_Data_cnt = 0;
			MST_Data_p = MST_Data;
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
//			SLV_Addr = I2C_Addr;              // Reset slave address
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
  
	transmit = 0;

	MST_Data = buff;
	MST_Data_p = buff;
}
