#define _SFR_ASM_COMPAT 1
#define __SFR_OFFSET 0

#include <avr/io.h>

.global LCD_show

LCD_show:
	rcall lcd_data_sim
	ret

	wait_msec:
		push r24
		push r25
		ldi r24, lo8(1000)
		ldi r25, hi8(1000)
		rcall wait_usec
		pop r25
		pop r24
		sbiw r24, 1
		brne wait_msec

		ret

	wait_usec:
		sbiw r24, 1 //2 cycles
		nop
		nop
		nop
		nop
		brne wait_usec //1 cycle the majority of the time

		ret

	write_2_nibbles_sim:
        push r24
        push r25
        ldi r24 ,lo8(6000) 
        ldi r25 ,hi8(6000)
        rcall wait_usec
        pop r25
        pop r24 
        push r24 
        in r25, PIND 
        andi r25, 0x0f 
        andi r24, 0xf0 
        add r24, r25 
        out PORTD, r24 
        sbi PORTD, PD3
        cbi PORTD, PD3
        push r24
        push r25
        ldi r24 ,lo8(6000)
        ldi r25 ,hi8(6000)
        rcall wait_usec
        pop r25
        pop r24
        pop r24
        swap r24
        andi r24 ,0xf0
        add r24, r25
        out PORTD, r24
        sbi PORTD, PD3
        cbi PORTD, PD3
        ret

	lcd_data_sim:
        push r24
        push r25
        sbi PORTD,PD2
        rcall write_2_nibbles_sim
        ldi r24,43
        ldi r25,0
        rcall wait_usec
        pop r25
        pop r24
        ret

	lcd_command_sim:
        push r24
        push r25
        cbi PORTD, PD2
        rcall write_2_nibbles_sim
        ldi r24, 39
        ldi r25, 0
        rcall wait_usec
        pop r25
        pop r24
        ret 

	lcd_init_sim:
        push r24
        push r25
        ldi r24, 40
        ldi r25, 0
        rcall wait_msec
        ldi r24, 0x30
        out PORTD, r24
        sbi PORTD, PD3
        cbi PORTD, PD3
        ldi r24, 39
        ldi r25, 0
        rcall wait_usec
        push r24
        push r25
        ldi r24,lo8(1000)
        ldi r25,hi8(1000)
        rcall wait_usec
        pop r25
        pop r24
        ldi r24, 0x30
        out PORTD, r24
        sbi PORTD, PD3
        cbi PORTD, PD3
        ldi r24,39
        ldi r25,0
        rcall wait_usec 
        push r24
        push r25
        ldi r24 ,lo8(1000)
        ldi r25 ,hi8(1000)
        rcall wait_usec
        pop r25
        pop r24
        ldi r24,0x20
        out PORTD, r24
        sbi PORTD, PD3
        cbi PORTD, PD3
        ldi r24,39
        ldi r25,0
        rcall wait_usec
        push r24
        push r25
        ldi r24 ,lo8(1000)
        ldi r25 ,hi8(1000)
        rcall wait_usec
        pop r25
        pop r24
        ldi r24,0x28
        rcall lcd_command_sim
        ldi r24,0x0c
        rcall lcd_command_sim
        ldi r24,0x01
        rcall lcd_command_sim
        ldi r24, lo8(1530)
        ldi r25, hi8(1530)
        rcall wait_usec
        ldi r24 ,0x06
        rcall lcd_command_sim
        pop r25
        pop r24
        ret
