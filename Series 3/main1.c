#undef F_CPU
#define F_CPU 8000000UL

#ifndef __DELAY_BACKWARD_COMPATIBLE__
#define __DELAY_BACKWARD_COMPATIBLE__
#endif

#include <avr/io.h>
#include <util/delay.h>

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

unsigned int buttons[2], ram[2];

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

int main(void)
{
    DDRC = 0xF0; //Initialize PORTC and LEDs. Internal resistor pull up must be deactivated
    DDRB = 0xFF;
    
    while(1) {
        unsigned char button1, button2;
        ram[0] = 0, ram[1] = 0;
        PORTB = 0x00;
        while(1) {
            if(scan_keypad_rising_edge_sim(15)) {
                button1 = keypad_to_ascii_sim();
                break;
            }
        }
        while(1) {
            if(scan_keypad_rising_edge_sim(15)) {
                button2 = keypad_to_ascii_sim();
                break;
            }
        }
        if ((button1 == '7') & (button2 == '1')) {
            PORTB = 0xFF;
            wait_msec(4000);
        }
        else {
            for (int i = 0; i < 4; i++) {
                PORTB = 0xFF;
                wait_msec(500);
                PORTB = 0x00;
                wait_msec(500);
            }
        }
    }
}
