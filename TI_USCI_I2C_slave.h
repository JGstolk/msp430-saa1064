#ifndef TI_USCI_I2C_SLAVE
#define TI_USCI_I2C_SLAVE

#define SDA_PIN 0x80                                  // msp430x261x UCB0SDA pin
#define SCL_PIN 0x40                                  // msp430x261x UCB0SCL pin

void TI_USCI_I2C_slaveinit(void (*SCallback)(unsigned char volatile *value),
                           void (*TCallback)(unsigned char volatile *value),
                           void (*RCallback)(unsigned char value), 
                           unsigned char slave_address);


#endif
