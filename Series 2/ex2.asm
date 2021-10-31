.org 0x0	;η αρχή του κώδικα reset
rjmp reset
.org 0x4	;η εξυπηρέτηση της ΙΝΤ1
rjmp ISR1

reset:
	ldi r24,(1<<ISC11)|(1<<ISC10)
	out MCUCR,r24	;η διακοπή ΙΝΤ1 να προκαλείται με σήμα θετικής ακμής
	ldi r24,(1<<INT1)
	out GICR,r24	;ενεργοποίηση διακοπής ΙΝΤ1
	sei
	ldi r24,low(RAMEND)
	out SPL,r24
	ldi r24,high(RAMEND)
	out SPH,r24


;counting programm given
start:
    clr r26
	out DDRA,r26	;Αρχικοποίηση της PORTA για είσοδο
	ser r26
	out DDRC,r26	;Αρχικοποίηση της PORTC για έξοδο
	out PORTA,r26	;ενεργοποίηση αντιστάσεων πρόσδεσης
	clr r16
	clr r26

loop:
	out PORTC,r26	;δείξε την τιμή του μετρητή στην θύρα εξόδου των LED
	inc r26
	rjmp loop

ISR1:
	push r26	;σώζει το περιεχόμενο του r26
	in r26,SREG	;σώζει το περιεχόμενο των sreg
	push r26
	ser r26
	out DDRB,r26	;η θύρα Β ως έξοδος
	in r26,PINA		;διαβάζει την Α
	subi r26,192
	brlo noincr		;αν δεν είναι 1 τα PA7 και PA6 πήγαινε στο noincr
	inc r16			;αλλιώς αύξησε τον μετρητή
	out PORTB,r16	;και βγάλτον στην έξοδο
noincr:
	pop r26		
	out SREG,r26	;επανάφερε την τιμη των sreg 
	pop r26			;και του r26
	reti			; και επίστρεψε