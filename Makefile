#CC - Contains the current C compiler. Defaults to cc. 
#CFLAGS - Special options which are added to the built-in C rule. (See next page.) 
#$@ - Full name of the current target. 
#$? - A list of files for current dependency which are out-of-date. 
#$< - The source file of the current (single) dependency. 

CC=msp430-gcc
CFLAGS=-Os -mmcu=msp430g2252
CFLAGS_USCI=-Os -mmcu=msp430g2553
LIBS=#-lm
OBJECTS=saa.o TI_USCI_I2C_slave.o led.o saa_usi.o saa_usci.o

saa: saa.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

TI_USCI_I2C_slave: TI_USCI_I2C_slave.o
	$(CC) $(CFLAGS_USCI) $^ $(LIBS) -o $@

led: led.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

saa_usci: saa_usci.o TI_USCI_I2C_slave.o led.o
	$(CC) $(CFLAGS_USCI) $^ $(LIBS) -o $@

saa_usi: saa_usi.o i2c_usi.o led.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf saa.elf *.o *~
