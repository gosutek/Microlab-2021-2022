#include <avr/io.h>
#include <avr/interrupt.h>

volatile int check, count, v;
volatile int bit_count(volatile int v);


ISR(INT0_vect) {
	
	check = PINA & 0x02;
	if (check == 0x00) {
		v = PINB;
		count = bit_count(v);
		count *= 2;
		PORTC = count + (count - 1);
	}
	else {
		v = PINB;
		count = bit_count(v);
		PORTC = count;
	}
}

volatile int bit_count(volatile int v) {
	volatile int count = 0x00;
	int hex_lsb;
	int loop_count = 7;
	while(loop_count > 0) {
		hex_lsb = v & 0x01;
		if (hex_lsb == 0x01) {
			count++;
		}
		v = v >> 1;
		loop_count--;
	}
	return count;
}


int main(void) {
	
	DDRA, DDRB, DDRD = 0x00; // make PORTA, PORTB, PORTD as inputs
	DDRC = 0xff;			// make PORTC as output
	
	MCUCR = 1<<INT0 | 1<<ISC00;
	GICR = 1<<INT0;
	
	while(1){};
}
