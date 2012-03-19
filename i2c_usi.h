#ifndef I2C_USI_H
#define I2C_USI_H

//#define __MSP430G2252__ 1
//#include <msp430.h>

//depreciated: #include <signal.h>
//#include <legacymsp430.h>

//#include <isr_compat.h>

//unsigned char MST_Data_cnt = 0;                          // Variable for received data
//unsigned char SLV_Data = 0x55;

//#define Number_of_Bytes  4                  // **** How many bytes?? ****

// make sure it is even address:
#define I2C_Addr 0x70
// Address is 0x38<<1 for R/W

//unsigned int I2C_State, Bytecount, transmit = 0;     // State variables

//void Data_RX(void);
//void TX_Data(void);

//interrupt(USI_VECTOR) usi_i2c_txrx(void);

//void Data_RX(void);

//void TX_Data(void);

void Setup_I2C(unsigned char* buff);

#endif
