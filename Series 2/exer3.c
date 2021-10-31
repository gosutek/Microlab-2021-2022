#include <avr/io.h>
#include <avr/interrupt.h>

volatile int check, count, v;
volatile int bit_count(volatile int v);


#define NOP(){__asm__ __volatile__("nop");} //does nothing. needed inside while(1) otherwise "step into" doesn't work.

// INT0 interrupt service routine
ISR(INT0_vect) {
	check = PINA & 0x04;	//get PA2
	if (check == 0x00) {	//if not set
		v = PINB;
		count = bit_count(v); //count the number of set bits
		PORTC = 4 * count - 1; //light up 'count' # of LEDs
	}
	else {
		v = PINB;
		count = bit_count(v);
		PORTC = count;	//output the number of set bits
	}
	return;
}

// function that counts the number of set bits in a byte
volatile int bit_count(volatile int v) {
	volatile int count = 0x00;
	int hex_lsb;
	int loop_count = 7;
	while(loop_count >= 0) {
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
	//disable global interrupts
	cli();
	
	GICR = 0x40;	//enable INT0
	MCUCR = 0x03;	// enable on rising edge
	//enable global interrupts
	sei();
	
	while(1) {NOP();}
}
