#undef F_CPU
#define F_CPU 8000000UL

#ifndef __DELAY_BACKWARD_COMPATIBLE__
#define __DELAY_BACKWARD_COMPATIBLE__
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>

#define NOP(){__asm__ __volatile__("nop");} //assembly nop

/*Keypad Functions*/
unsigned int scan_row_sim(unsigned int row);
void scan_keypad_sim();
unsigned int scan_keypad_rising_edge_sim(unsigned int flick_time);
unsigned char keypad_to_ascii_sim();

unsigned int swap(unsigned int val);

/*Delay functions*/
void wait_usec(unsigned int delay);
void wait_msec(unsigned int delay);

/*LCD Assembly*/
//we kept the assembly code, given by the proffessors, for these lcd functions and we imported them from the .S files 
extern void LCD_init();
extern void LCD_show(unsigned char cha);

/*Globals*/
unsigned int buttons[2], ram[2];
volatile unsigned int adc_output = 0;

/*function that delays for delay time*/
void wait_usec(unsigned int delay){
	unsigned int i;
	for(i = 0; i < (delay/10); i++) {    //10 usec delay for delay/10 times
		_delay_us(10);
	}
	if (delay % 10) {    //delay for the remainder accordingly
		_delay_us(delay % 10);
	}
}
/*same function as wait_usec but for milliseconds*/
void wait_msec(unsigned int delay) {
	unsigned int i ;
	for(i = 0; i < (delay / 10); i++){
		_delay_ms(10);
	}
	if(delay % 10) {
		_delay_ms(delay % 10);
	}
}
/*Function that does __asm__("swap")*/
unsigned int swap(unsigned int val) {
	return ((val & 0x0F) << 4 | (val & 0xF0) >> 4);
}

/*Function that returns which columns are pressed for a given row*/
unsigned int scan_row_sim(unsigned int row) {
	PORTC = row;//Search in line row.
	wait_usec(500); //Delay required for a successful remote operation
	
	NOP();
	NOP();//Delay to allow for a change of state
	
	return PINC & 0x0F; //Return 4 LSB
}
/*Function that scans the whole keypad*/
void scan_keypad_sim() {
	buttons[1] = swap(scan_row_sim(0x10));// A 3 2 1
	buttons[1] += scan_row_sim(0x20);// A 3 2 1 B 6 5 4

	buttons[0] = swap(scan_row_sim(0x40));// C 9 8 7
	buttons[0] += (scan_row_sim(0x80)); // C 9 8 7 D # 0 *

	PORTC = 0x00; // added only for the remote operation
	return;
}
/*Function that checks which buttons where pressed since its last call*/
unsigned int scan_keypad_rising_edge_sim(unsigned int flick_time) {
	scan_keypad_sim(); // do the first scan
	unsigned int temp[2]; // store first scan
	temp[0] = buttons[0];
	temp[1] = buttons[1];
	wait_msec(flick_time); //wait for flick time
	
	scan_keypad_sim();  //scan second time
	buttons[0] &= temp[0]; //remove flick values
	buttons[1] &= temp[1];

	temp[0] = ram[0];   //get the last state from previous call to rising_edge from "RAM"
	temp[1] = ram[1];
	ram[0] = buttons[0];    //update the new previous state
	ram[1] = buttons[1];
	buttons[0] &= ~temp[0]; //Keep values that change from 1 to 0
	buttons[1] &= ~temp[1];
	
	return (buttons[0] || buttons[1]);
}
/*Function that returns the ASCII code of button pressed*/
unsigned char keypad_to_ascii_sim() {
	unsigned int select;
	for(select = 0x01; select <= 0x80; select <<= 1) {
		switch(buttons[0] & select) {
			case 0x01:
			return '*';
			case 0x02:
			return '0';
			case 0x04:
			return '#';
			case 0x08:
			return 'D';
			case 0x10:
			return '7';
			case 0x20:
			return '8';
			case 0x40:
			return '9';
			case 0x80:
			return 'C';
		}
	}
	for(select = 0x01; select <= 0x80; select <<= 1) {
		switch(buttons[1] & select) {
			case 0x01:
			return '4';
			case 0x02:
			return '5';
			case 0x04:
			return '6';
			case 0x08:
			return 'B';
			case 0x10:
			return '1';
			case 0x20:
			return '2';
			case 0x40:
			return '3';
			case 0x80:
			return 'A';
		}
	}
	return 0;
}

void PWM_init() {
	//set TMR0 in fast PWM mode with non-inverted output, prescale=8
	TCCR0 = (1<<WGM00) | (1<<WGM01) | (1<<COM01) | (1<<CS01);
	DDRB |= (1<<PB3);
}

void ADC_init(){
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

ISR(TIMER1_OVF_vect){
	TCNT1 = 64755; //100ms
	ADCSRA |= (1<<ADSC);//adc start conversion enable
}

ISR(ADC_vect){
	adc_output = ADC;
}

//function which takes the ADC and shows in the lcd the converted voltage value
void convert_to_voltage_and_show(char adc_decimal_buffer[]) {
	float analog_input = (float) 5 * adc_output / 1024;
	
	char integer_part = (int) analog_input + '0';
	unsigned int decimal_part = (int) (analog_input * 100) % 100;
	itoa(decimal_part, adc_decimal_buffer, 10);
	
	LCD_show(integer_part);
	LCD_show('.');
	LCD_show(adc_decimal_buffer[0]);
	LCD_show(adc_decimal_buffer[1]);
	
	return;
}


int main(void)
{
	DDRD = 0xFF;	//Initialize LCD as output
	DDRC = 0xF0;	//Initialize PORTC and LEDs. Internal resistor pull up must be deactivated
	unsigned int duty = 0;
	char adc_decimal[2];
	unsigned int old_output = -1;
	ADC_init();
	LCD_init();
	PWM_init();
	TIMSK = (1<<TOIE1);
	TCCR1B = (1<<CS12)|(0<<CS11)|(1<<CS10);
	TCNT1 = 64755; //100ms
	sei();
	while(1) {
		unsigned char button_pressed;
		ram[0] = 0, ram[1] = 0;
		while(1) {
			if(scan_keypad_rising_edge_sim(15)) {
				button_pressed = keypad_to_ascii_sim();
				break;
			}
			if (old_output != adc_output) { //if the adc hasn't changed, dont show and compute the output again
				old_output = adc_output;
				LCD_init();
				LCD_show('V');
				LCD_show('o');
				LCD_show('1');
				LCD_show('\n');
				convert_to_voltage_and_show(adc_decimal);
			}
		}
		if ((button_pressed == '1') && duty < 255) {
			duty++;//increasing the upper pulse
			OCR0 = duty;
		}
		else if((button_pressed == '2') && duty > 0){
			duty--;//decreasing the upper pulse
			OCR0 = duty;
		}
	}
}
