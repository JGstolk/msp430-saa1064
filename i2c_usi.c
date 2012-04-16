
#define __MSP430G2252__ 1
#include <msp430.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

#include "i2c_usi.h"
#include "led.h"

//unsigned char MST_Data[4] = { 0x00, 0x00, 0x00, 0x00 };
unsigned char* MST_Data = 0;
unsigned char* SLV_Data = 0;

unsigned int I2C_State, Bytecount, int_count, rx_count, start_count, restart_count, stop_count;     // State variables

const unsigned char hex2seven_matrix[] = { SEG_ZERO,
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

#define SCL (P1IN & BIT6)

//******************************************************************************
// USI interrupt service routine
// Rx bytes from master: State 2->4->6->8 
// Tx bytes to Master: State 2->4->10->12->14
//******************************************************************************
interrupt(USI_VECTOR) usi_i2c_txrx(void)
{
	if (USICTL1 & USISTTIFG) {                 // Start entry?
		I2C_State = 2;                          // Enter 1st state on start
		if (!USICTL1 & USISTP)
			++restart_count;
		++start_count;
	}

	if (USICTL1 & USISTP) {
		++stop_count;
		USICTL1 &= ~USISTP;
	}

//	USICTL1 &= ~USIIFG;                       // Clear pending flags

	switch(I2C_State) {

	case 2: // RX Address
		// USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, RX address
		// // USISCLREL | USI16B | USIIFGCC
		while(SCL);
		USICNT &= 0xE0;
		USICNT |= 0x08;               // Bit counter = 8, RX address
		USICTL1 &= ~USISTTIFG;        // Clear start flag
		I2C_State = 4;                // Go to next state: check address
		break;

	case 4: // Process Address and send (N)Ack

		if ( (USISRL & 0xFE) == I2C_Addr)       // Address match?
		{
			I2C_State = (USISRL & 0x01) ? 10 : 6;
			while(SCL);
			USISRL = 0x00;              // Send Ack
			USICNT |= 0x01;               // Bit counter = 1, send (N)Ack bit
			USICTL0 |= USIOE;             // SDA = output
			Bytecount = 0;                // Reset counter for next TX/RX
		} else
			I2C_State = 2;              // next state: prep for next Start
			Bytecount = 0;                // Reset counter for next TX/RX
		break;

	case 6: // Receive data byte

//		MST_Data[0x3 & Bytecount] = USISRL;

		USICTL0 &= ~USIOE;            // SDA = input
		while(SCL);
		USICNT |= 0x08;              // Bit counter = 8, RX data
		I2C_State = 8;                // next state: Test data and (N)Ack
//		rx_count++;
		break;

	case 8:// Check Data & TX (N)Ack
		if (Bytecount <= 4 ) { // expected number of bytes // If not last byte
//			if(Bytecount < 4)
//				MST_Data[0x3 & Bytecount] = USISRL;
			Bytecount++;

//			if(start_count == 1)
//				MST_Data[1] = hex2seven_matrix[USISRL & 0xf]; //  1 / 2 / 3 /  4 /   5 /

			USICTL0 |= USIOE;             // SDA = output
			USISRL = 0x00;              // Send Ack
			I2C_State = 6;              // Rcv another byte
			while(SCL);
			USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
		} else {                          // Last Byte
			USICTL0 |= USIOE;             // SDA = output
			USISRL = 0xFF;              // Send NAck
			USICTL0 &= ~USIOE;            // SDA = input
			I2C_State = 0;                // Reset state machine
			Bytecount = 0;                // Reset counter for next TX/RX
		}
		break;

	case 10: // Send Data byte A
		USICTL0 |= USIOE;             // SDA = output
		USISRL = SLV_Data[Bytecount++];
//		USISRL = SLV_Data[1];
//		USISRL = Bytecount;
		while(SCL);
		USICNT |= 0x08;              // Bit counter = 8, TX data
		I2C_State = 12;               // Go to next state: receive (N)Ack
		break;

	case 12:// Receive Data (N)Ack C
		USICTL0 &= ~USIOE;            // SDA = input
		while(SCL);
		USICNT |= 0x01;               // Bit counter = 1, receive (N)Ack
		I2C_State = 14;               // Go to next state: check (N)Ack
		break;

	case 14:// Process Data Ack/NAck E
		if (USISRL & 0x01)               // If Nack received...
		{
			USICTL0 &= ~USIOE;            // SDA = input
			I2C_State = 0;                // Reset state machine
			Bytecount = 0;
			// LPM0_EXIT;                  // Exit active for next transfer
		} else {                         // Ack received
			USICTL0 |= USIOE;             // SDA = output
			USISRL = SLV_Data[Bytecount++];
//			USISRL = SLV_Data[1];
//			USISRL = Bytecount;
			while(SCL);
			USICNT |= 0x08;              // Bit counter = 8, TX data
			I2C_State = 12;               // Go to next state: receive (N)Ack
		}
		break;
	default:
	case 0:                               // Idle, should not get here
		break;
	}

	USICTL1 &= ~USIIFG;                       // Clear pending flags

//	if(USICTL1 & USIIFG) {
//		MST_Data[2] = hex2seven_matrix[I2C_State & 0xf]; //  8 / 8 / 8 /  8 /   8 /
//		MST_Data[3] = 0xff;
//	}


//	if(int_count < 4)
//		MST_Data[int_count] = hex2seven_matrix[I2C_State & 0xf];

	int_count++;

//                                                           - / b / w / wp / saa /
	MST_Data[0] = hex2seven_matrix[int_count & 0xf]; //  5 / 7 / 9 / 11 /  13 /

//	MST_Data[1] = hex2seven_matrix[Bytecount & 0xf]; //  1 / 2 / 3 /  4 /   5 /

//	MST_Data[2] = hex2seven_matrix[I2C_State & 0xf]; //  8 / 8 / 8 /  8 /   8 /

//	MST_Data[3] = hex2seven_matrix[ rx_count & 0xf]; //  1 / 2 / 3 /  4 /   5 /

//                                                             -   /   b / w   /   wp / saa /   gb / gw
	MST_Data[1] = hex2seven_matrix[start_count & 0xf]; //  1+1 / 1+1 / 1+1 /  1+1 / 1+1 /  1+1 /    /
	MST_Data[2] = hex2seven_matrix[stop_count & 0xf];  //  0+1 / 0+1 / 0+1 /  0+1 / 0+1 /  0+1 /    /
	MST_Data[3] = hex2seven_matrix[start_count & 0xf]; //  1+1 / 1+1 / 1+1 /  1+1 / 1+1 /  1+1 /    /
}

void Setup_I2C(unsigned char* led_buff, unsigned char* adc_buff){

	P1DIR |= (BIT6 | BIT7);
	P1OUT |= (BIT6 | BIT7);             // P1.6 & P1.7 Pullups / P1.3 segment C
	P1REN |= (BIT6 | BIT7);             // P1.6 & P1.7 Pullups

// USIPE7              (0x80)    /* USI  Port Enable Px.7 */
// USIPE6              (0x40)    /* USI  Port Enable Px.6 */
// USIPE5              (0x20)    /* USI  Port Enable Px.5 */
// USILSB              (0x10)    /* USI  LSB first  1:LSB / 0:MSB */
// USIMST              (0x08)    /* USI  Master Select  0:Slave / 1:Master */
// USIGE               (0x04)    /* USI  General Output Enable Latch */
// USIOE               (0x02)    /* USI  Output Enable */
// USISWRST            (0x01)    /* USI  Software Reset */
	USICTL0 = USIPE6+USIPE7+USISWRST;         // Port & USI mode setup

// USICKPH             (0x80)    /* USI  Sync. Mode: Clock Phase */
// USII2C              (0x40)    /* USI  I2C Mode */
// USISTTIE            (0x20)    /* USI  START Condition interrupt enable */
// USIIE               (0x10)    /* USI  Counter Interrupt enable */
// USIAL               (0x08)    /* USI  Arbitration Lost */
// USISTP              (0x04)    /* USI  STOP Condition received */
// USISTTIFG           (0x02)    /* USI  START Condition interrupt Flag */
// USIIFG              (0x01)    /* USI  Counter Interrupt Flag */
	USICTL1 = USII2C+USIIE+USISTTIE;          // Enable I2C mode & USI interrupts

	USICKCTL = USICKPL;                       // Setup clock polarity

// USISCLREL           (0x80)    /* USI  SCL Released */
// USI16B              (0x40)    /* USI  16 Bit Shift Register Enable */
// USIIFGCC            (0x20)    /* USI  Interrupt Flag Clear Control */
// USICNT4             (0x10)    /* USI  Bit Count 4 */
// USICNT3             (0x08)    /* USI  Bit Count 3 */
// USICNT2             (0x04)    /* USI  Bit Count 2 */
// USICNT1             (0x02)    /* USI  Bit Count 1 */
// USICNT0             (0x01)    /* USI  Bit Count 0 */
	USICNT |= USIIFGCC;                       // Disable automatic clear control

	USICTL0 &= ~USISWRST;                     // Enable USI
	USICTL1 &= ~USIIFG;                       // Clear pending flag
  
	SLV_Data = (unsigned char*)adc_buff;
	MST_Data = (unsigned char*)led_buff;
}
