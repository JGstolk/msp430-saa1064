#CC - Contains the current C compiler. Defaults to cc. 
#CFLAGS - Special options which are added to the built-in C rule. (See next page.) 
#$@ - Full name of the current target. 
#$? - A list of files for current dependency which are out-of-date. 
#$< - The source file of the current (single) dependency. 

CC=msp430-gcc
#CFLAGS=-Os -mmcu=msp430g2231
CFLAGS=-Os -mmcu=msp430g2452
LIBS=#-lm
OBJECTS=saa.elf

saa: saa.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf saa.elf *.o *~
