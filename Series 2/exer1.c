#include <avr/io.h>

//Initialize variables
char F0,F1,A,B,C,D;

int main() {
	
	//Set PORTC as input
	DDRC = 0x00;
	//Set PORTB as output
	DDRB = 0xFF;
	
	while(1) {
		
		A = PINC & 0x01;		//Get PC0
		B = (PINC & 0x02) >> 1; //Get PC1 and shift to LSB
		C = (PINC & 0x04) >> 2;	//Get PC2 and shift to LSB
		D = (PINC & 0x08) >> 3;	//Get PC3 and shift to LSB
		
		//Perform bitwise operation for F0 = (A'B + B'CD)'
		F0 = ~((~A & B) | (~B & C & D));
		//Keep the LSB
		F0 = F0 & 0x01;
		
		//Perform bitwise operation for F1 = (AC)(B + D)
		F1 = (A & C) & (B | D);
		//Keep the LSB
		F1 = F1 & 0x01;
		//Shift one to the left
		F1 = F1 << 1;
		//Merge F0-F1 and output
		PORTB = F0 | F1;
		
	}
	
	
}
