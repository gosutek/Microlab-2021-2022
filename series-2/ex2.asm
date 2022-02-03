.org 0x0	;� ���� ��� ������ reset
rjmp reset
.org 0x4	;� ����������� ��� ���1
rjmp ISR1

reset:
	ldi r24,(1<<ISC11)|(1<<ISC10)
	out MCUCR,r24	;� ������� ���1 �� ����������� �� ���� ������� �����
	ldi r24,(1<<INT1)
	out GICR,r24	;������������ �������� ���1
	sei
	ldi r24,low(RAMEND)
	out SPL,r24
	ldi r24,high(RAMEND)
	out SPH,r24


;counting programm given
start:
    clr r26
	out DDRA,r26	;������������ ��� PORTA ��� ������
	ser r26
	out DDRC,r26	;������������ ��� PORTC ��� �����
	out PORTA,r26	;������������ ����������� ���������
	clr r16
	clr r26

loop:
	out PORTC,r26	;����� ��� ���� ��� ������� ���� ���� ������ ��� LED
	inc r26
	rjmp loop

ISR1:
	push r26	;����� �� ����������� ��� r26
	in r26,SREG	;����� �� ����������� ��� sreg
	push r26
	ser r26
	out DDRB,r26	;� ���� � �� ������
	in r26,PINA		;�������� ��� �
	subi r26,192
	brlo noincr		;�� ��� ����� 1 �� PA7 ��� PA6 ������� ��� noincr
	inc r16			;������ ������ ��� �������
	out PORTB,r16	;��� ������� ���� �����
noincr:
	pop r26		
	out SREG,r26	;��������� ��� ���� ��� sreg 
	pop r26			;��� ��� r26
	reti			; ��� ���������