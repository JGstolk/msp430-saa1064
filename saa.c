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
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//            |             P1.6|------->|SEG A            |
//             -----------------          -----------------
//
//******************************************************************************

#define __MSP430G2252__ 1
//#define __MSP430G2231__ 1
#include <msp430.h>

//#include <msp430g2231.h>
//#include <msp430g2452.h>

//depreciated: #include <signal.h>
#include <legacymsp430.h>

#include <isr_compat.h>

unsigned char status = 0x01, dir = 0x01;
unsigned char p = 0x80;

unsigned char d = 0;
typedef union {
	unsigned char byte;
	struct
	{
		unsigned char seg_a : 1;
		unsigned char seg_b : 1;
		unsigned char seg_c : 1;
		unsigned char seg_d : 1;
		unsigned char seg_e : 1;
		unsigned char seg_f : 1;
		unsigned char seg_g : 1;
		unsigned char seg_h : 1;
	};
} digit_t;

digit_t digits[4];

#define SEGA 0b00000001
#define SEGB 0b00000010
#define SEGC 0b00000100
#define SEGD 0b00001000
#define SEGE 0b00010000
#define SEGF 0b00100000
#define SEGG 0b01000000
#define SEGH 0b10000000

#define SEG_ZERO (SEGA | SEGB | SEGC | SEGD | SEGE | SEGF)
#define SEG_ONE (SEGB | SEGC)
#define SEG_TWO (SEGA | SEGB | SEGG | SEGE | SEGD)
#define SEG_THREE (SEGA | SEGB | SEGG | SEGC | SEGD)
#define SEG_FOUR (SEGF | SEGB | SEGG | SEGC)
#define SEG_FIVE (SEGA | SEGF | SEGG | SEGC | SEGD)
#define SEG_SIX (SEGA | SEGF | SEGE | SEGC | SEGD | SEGG)
#define SEG_SEVEN (SEGA | SEGB | SEGC)
#define SEG_EIGHT (SEGA | SEGB | SEGC | SEGD | SEGE | SEGF)
#define SEG_NINE (SEGA | SEGB | SEGC | SEGD | SEGG | SEGF)

#define SEG_AA (SEGA | SEGB | SEGC | SEGG | SEGE | SEGF)
#define SEG_BB (SEGC | SEGD | SEGG | SEGE | SEGF)
#define SEG_CC (SEGA | SEGD | SEGE | SEGF)
#define SEG_DD (SEGB | SEGC | SEGD | SEGE | SEGG)
#define SEG_EE (SEGA | SEGD | SEGG | SEGE | SEGF)
#define SEG_FF (SEGA | SEGE | SEGF | SEGG)

#define Number_of_Bytes  4                  // **** How many bytes?? ****

void Setup_USI_Slave(void);

char MST_Data = 0;                          // Variable for received data
char SLV_Data = 0x55;
char SLV_Addr = 0x90;                       // Address is 0x48<<1 for R/W
int I2C_State, Bytecount, transmit = 0;     // State variables

void Data_RX(void);
void TX_Data(void);

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

	P1DIR |= (BIT0 | BIT6);
	P1OUT = BIT0;

	TACTL |= MC_1;
	TACTL |= TASSEL_1;
	TACTL |= TAIE;

	BCSCTL3 |= LFXT1S_2;

	TACCTL0 = CCIE;

	TACCR0 = 0x40; //0x0fff;  //SMCLK/TIME_1MS;

	digits[0].byte = SEG_ZERO;
	digits[1].byte = SEG_ONE;
	digits[2].byte = SEG_TWO;
	digits[3].byte = SEG_THREE;

	eint();

	for (;;)
	{
		nop();
		WRITE_SR(GIE | CPUOFF);
	}
}

//ISR(TIMER0_A0,timer0_a3_isr)
interrupt(TIMER0_A0_VECTOR) timer0_a3_isr(void)
{
// clear anodes:
	P1OUT &= ~(BIT2 | BIT4 | BIT5);
	P2OUT &= ~(BIT2);

// clear cathodes:
	P1OUT |= (BIT3);
	P2OUT |= (BIT0 | BIT1 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

// set next digit

	++d;
	d &= 0b11;

	switch (d) {
	case 0b11:
		P1OUT |= BIT2;
		break;
	case 0b10:
		P1OUT |= BIT4;
		break;
	case 0b01:
		P1OUT |= BIT5;
		break;
	case 0b00:
		P2OUT |= BIT2;
		break;
	default:
		break;
	}
// output appropriate digit on cathode

	P1OUT &= ~( digits[d].seg_b ? BIT3:0x0 );
	P2OUT &= ~(
		(digits[d].seg_f ? BIT0:0x0) |
		(digits[d].seg_a ? BIT1:0x0) |
		(digits[d].seg_e ? BIT3:0x0) |
		(digits[d].seg_d ? BIT4:0x0) |
		(digits[d].seg_h ? BIT5:0x0) |
		(digits[d].seg_c ? BIT7:0x0) |
		(digits[d].seg_g ? BIT6:0x0));

 	TACTL &= ~TAIFG;
};

//******************************************************************************
// USI interrupt service routine
// Rx bytes from master: State 2->4->6->8 
// Tx bytes to Master: State 2->4->10->12->14
//******************************************************************************
//ISR(USI,usi_i2c_isr)
interrupt(USI_VECTOR) usi_i2c_txrx(void)
{
	if (USICTL1 & USISTTIFG)                  // Start entry?
		I2C_State = 2;                          // Enter 1st state on start

//	switch(__even_in_range(I2C_State,14))
	switch(I2C_State) {
	{
	case 0:                               // Idle, should not get here
		break;

	case 2: // RX Address
		USICNT = (USICNT & 0xE0) + 0x08; // Bit counter = 8, RX address
		USICTL1 &= ~USISTTIFG;        // Clear start flag
		I2C_State = 4;                // Go to next state: check address
		break;

	case 4: // Process Address and send (N)Ack
		if (USISRL & 0x01){            // If master read...
			SLV_Addr = 0x91;             // Save R/W bit
			transmit = 1;}
		else{transmit = 0;
			SLV_Addr = 0x90;}
		USICTL0 |= USIOE;             // SDA = output
		if (USISRL == SLV_Addr)       // Address match?
		{
			USISRL = 0x00;              // Send Ack
			if (transmit == 0){ 
				I2C_State = 6;}           // Go to next state: RX data
			if (transmit == 1){  
				I2C_State = 10;}          // Else go to next state: TX data
		}
		else
		{
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
		if (Bytecount <= (Number_of_Bytes-2))          
			// If not last byte
		{
			USISRL = 0x00;              // Send Ack
			I2C_State = 6;              // Rcv another byte
			Bytecount++;
			USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
		}
		else                          // Last Byte
		{
			USISRL = 0xFF;              // Send NAck
			USICTL0 &= ~USIOE;            // SDA = input
			SLV_Addr = 0x90;              // Reset slave address
			I2C_State = 0;                // Reset state machine
			Bytecount =0;                 // Reset counter for next TX/RX
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
			SLV_Addr = 0x90;              // Reset slave address
			I2C_State = 0;                // Reset state machine
			Bytecount = 0;
			// LPM0_EXIT;                  // Exit active for next transfer
		}
		else                          // Ack received
			TX_Data();                  // TX next byte
		break;
	}
	USICTL1 &= ~USIIFG;                       // Clear pending flags
}

void Data_RX(void){
	digits[Bytecount - 2].byte = USISRL;
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

void Setup_USI_Slave(void){

	P1DIR |= (              BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
	P2DIR |= (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7);

	P1OUT = (              BIT3 |              BIT6 | BIT7);
	P2OUT = (BIT0 | BIT1 | BIT3 | BIT4 | BIT5);

	P1OUT = (BIT3 | BIT6 | BIT7);             // P1.6 & P1.7 Pullups / P1.3 segment C
	P1REN |= 0xC0;                            // P1.6 & P1.7 Pullups

	P2SEL &= ~(BIT6 | BIT7); // changes the function of XIN/XOUT into GPIO
  
	USICTL0 = USIPE6+USIPE7+USISWRST;         // Port & USI mode setup
	USICTL1 = USII2C+USIIE+USISTTIE;          // Enable I2C mode & USI interrupts
	USICKCTL = USICKPL;                       // Setup clock polarity
	USICNT |= USIIFGCC;                       // Disable automatic clear control
	USICTL0 &= ~USISWRST;                     // Enable USI
	USICTL1 &= ~USIIFG;                       // Clear pending flag
  
	transmit = 0;
	_EINT();
}
