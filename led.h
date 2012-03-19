#ifndef LED_H
#define LED_H

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

digit_t* Setup_LED(void);

void Init_display(void);

#endif
