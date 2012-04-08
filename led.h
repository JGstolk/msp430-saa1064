#ifndef LED_H
#define LED_H

#define SEGA 0b00000001
#define SEGB 0b00000010
#define SEGC 0b00000100
#define SEGD 0b00001000
#define SEGE 0b00010000
#define SEGF 0b00100000
#define SEGG 0b01000000
#define SEGH 0b10000000

#define SEG_BLANK (0x00)
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

unsigned char* Setup_LED(void);

void Init_display(void);

#endif
