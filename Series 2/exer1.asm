

.org 0x0
rjmp reset

reset:
	; set r24 to 0x00
	clr r24
	; set PORTC as input
	out DDRC , r24
	; set r24 to 0xFF
	ser r24
	;set PORTB as output
	out DDRB , r24

main:
	;r16 = F0
	;r17 = F1
	;r18 = A
	;r19 = B
	;r20 = C
	;r21 = D

	in r18 , PINC	// get the input
	andi r18, 0x01	//get PC0

	in r19, PINC	//get the input
	andi r19, 0x02	// get PC1
	lsr r19			//shift right

	in r20, PINC
	andi r20, 0x04
	lsr r20
	lsr r20

	in r21, PINC
	andi r21, 0x08
	lsr r21
	lsr r21
	lsr r21

	; F0 = (A'B + B'CD)'

	mov r24, r18 ; r24 <- A
	com r24 ; r24 <- A'
	and r24, r19 ; r24 <- A'B
	mov r16, r24 ; F0 <- A'B

	mov r24, r19 ; r24 <- B
	com r24		;r24 <- B'
	and r24, r20	; r24 <- B'C
	and r24, r21	; r24 <- B'CD

	or r16, r24		; F0 <- (A'B + B'CD)
	com r16			; F0 <- (A'B + B'CD)'
	andi r16, 0x01	; get the lsb

	; F1 = (AC)(B + D)

	mov r24, r18	;r24 <- A
	and r24, r20	; r24 <- AC
	mov r17, r24	; F1 <- AC

	mov r24, r19	;r24 <- B
	or r24, r21		;r24 <- B + D

	and r17, r24	; F1 <- (AC)(B + D)
	andi r17, 0x01	; get the lsb
	lsl r17			; shift left

	or r16, r17
	out PORTB, r16	;show on LEDs

	rjmp main
