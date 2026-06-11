/*
 * GccApplication3.c
 *
 * Created: 5/19/2025 4:37:11 PM
 * Author : shema
 */ 

#define F_CPU 1000000
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Image to display
volatile unsigned char image[8] =
{
	0b00000111,
	0b01111011,
	0b01111101,
	0b01111101,
	0b01111101,
	0b01111101,
	0b01111011,
	0b00000111
};




volatile unsigned char column_selector = 0b10000000;   // determines which column of the 8x8 LED matrix is currently active

volatile uint8_t toggle_scroll = 0;     // Toggle row scrolling
volatile uint8_t toggle_shift_bits = 0; // Toggle column shifting (left/right)

// === INT0: Toggle row scrolling ===
ISR(INT0_vect) {
	toggle_scroll = ~toggle_scroll;
	toggle_shift_bits = 0;
}

// === INT1: Toggle column shift direction ===
ISR(INT1_vect) {
	toggle_shift_bits = ~toggle_shift_bits;
	toggle_scroll = 0;
}

// === Row control ===
unsigned char get_next_column_selector_down(unsigned char current_column_selector) {
	unsigned char next = current_column_selector >> 1;
	if (!next) next = 0b10000000;
	return next;
}

unsigned char get_next_column_selector_up(unsigned char current_column_selector) {
	unsigned char next = current_column_selector << 1;
	if (!next) next = 0b00000001;
	return next;
}

void shift_image_left() {
	// Right-to-left shift (horizontal)
	for (int i = 0; i < 8; i++) {
		image[i] = (image[i] << 1) | ((image[i] & 0x80) >> 7); // rotate left
	}
}

void shift_image_right() {
	for (int i = 0; i < 8; i++) {
		unsigned char lsb = image[i] & 0x01; // store bit 0
		image[i] = (image[i] >> 1) | (lsb << 7); // rotate right
	}
}



void displayFrame() {
	PORTA = column_selector;
	for (int i = 0; i < 8; i++) {
		PORTB = image[i];
		_delay_ms(0.5);
		PORTA = get_next_column_selector_down(PORTA);
	}
}

int main(void) {
	DDRA = 0xFF;
	DDRB = 0xFF;
	DDRD = 0x00; // Set PD2 and PD3 as input for INT0 and INT1

	// Enable INT0 (PD2) and INT1 (PD3)
	GICR |= (1 << INT0) | (1 << INT1);
	MCUCR = MCUCR ^ MCUCR;
	MCUCR |= (1 << ISC00) | (1 << ISC10);
	sei();

	while (1) {
		
		for (int i = 0; i < 50; i++)
		displayFrame();


		if (toggle_scroll & 0x01)
		column_selector = get_next_column_selector_down(column_selector);
		

		if (toggle_shift_bits & 0x01)
		shift_image_right();
		
	}
}
