#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define NOP()                        \
    {                                \
        __asm__ __volatile__("nop"); \
    }

int temp = 0x0000;
int t_flag = 0;                  // όταν είναι 0 βγαζει στην εξοδο 0, όταν είναι 1 βγάζει το επίπεδο του αερίου
int password = 0;                //αν είναι 1 (σωστός κωδικός) κάνει το t_flag 1, για να μην αναβοσβήνουν τα 4 δευτερόλεπτα που η ειδική ομάδα είναι στην αίθουσα
int leds = 0x00, MSB_led = 0x00; //η κατάσταση του επιπέδου του αερίου και του MLSB της portb

ISR(TIMER1_OVF_vect)
{
    TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10);
    TCNT1 = 64755; // 100ms
    ADCSRA |= (1 << ADSC);

    if (t_flag == 0)
        t_flag = 1; //ανα 100ms αλλάζουμε αυτήν την τιμή για να αναβοσβήσουμε τα led όταν το επίπεδο του αερίου ξεπεράσει τα όρια
    else
        t_flag = 0;
    if (password == 1)
        t_flag = 1; //σε περίπτωση σωστού κωδικού σταματάμε την εναλλαγή
}

//Απο τον τύπο Vgas = C*M + Vgas0 βρήκαμε οτι για C = 70ppm, Vgas = 1.
//Επιπλέον η έξοδος του ADC είναι ADC = Vin * (1024/5) = 204.8*Vin, όπου Vin = Vgas.
//Άρα, έχουμε συναγερμό για ADC>205
//Χωρίζουμε τα επίπεδα σε 7, άρα 1024/7 = 147 τιμές το επίπεδο

ISR(ADC_vect)
{
    if (ADC < 147)
        PORTB = 0x01 | (MSB_led << 7);
    else if (ADC >= 147 && ADC < 205)
        PORTB = 0x03 | (MSB_led << 7);
    else if (t_flag == 1)
    {
        if (ADC >= 205 && ADC < 294)
            leds = 0x03;
        else if (ADC >= 294 && ADC < 441)
            leds = 0x07;
        else if (ADC >= 441 && ADC < 588)
            leds = 0x0F;
        else if (ADC >= 588 && ADC < 735)
            leds = 0x1F;
        else if (ADC >= 735 && ADC < 882)
            leds = 0x3F;
        else if (ADC >= 882)
            leds = 0x7F;
        PORTB = leds | (MSB_led << 7);
    }
    else
    {
        PORTB = MSB_led << 7;
    }
}

void adc_init()
{
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

int scan_row_sim(int r25)
{
    PORTC = r25;
    int r24;
    _delay_us(500);
    NOP();
    NOP();
    r24 = PINC;
    r24 = r24 & 0x0F;
    return r24;
}

// a is LSB, b is MSB
void scan_keypad_sim(int *a, int *b)
{

    int r25 = 0x10, r24, r27, r26;
    r24 = scan_row_sim(r25);
    r27 = ((r24 & 0x0F) << 4 | (r24 & 0xF0) >> 4); // swap r24

    r25 = 0x20;
    r24 = scan_row_sim(r25);
    r27 += r24;

    r25 = 0x40;
    r24 = scan_row_sim(r25);
    r26 = ((r24 & 0x0F) << 4 | (r24 & 0xF0) >> 4); // swap r24

    r25 = 0x80;
    r24 = scan_row_sim(r25);
    r26 += r24;
    *a = r26;
    *b = r27;

    PORTC = 0x00;
    return;
}

void scan_keypad_rising_edge_sim(int time, int *a, int *b)
{
    int r22, r23, r24, r25;
    int r26;
    scan_keypad_sim(&r24, &r25);
    _delay_ms(15);
    scan_keypad_sim(&r22, &r23);
    r24 = r24 & r22;
    r25 = r25 & r23;
    r26 = temp;
    r25 = r25 << 8;
    temp = r25 | r24;
    r26 = ~r26;
    *a = r24 & (r26 & 0x00FF);
    *b = (r25 >> 8) & ((r26 & 0xFF00) >> 8);

    return;
}

int keypad_to_ascii_sim(int r24, int r25)
{
    int r26 = '0';
    switch (r24)
    {
    case 0x01:
        r26 = '*';
        break;
    case 0x02:
        r26 = '0';
        break;
    case 0x04:
        r26 = '#';
        break;
    case 0x08:
        r26 = 'D';
        break;
    case 0x10:
        r26 = '7';
        break;
    case 0x20:
        r26 = '8';
        break;
    case 0x40:
        r26 = '9';
        break;
    case 0x80:
        r26 = 'C';
        break;
    }
    switch (r25)
    {
    case 0x01:
        r26 = '4';
        break;
    case 0x02:
        r26 = '5';
        break;
    case 0x04:
        r26 = '6';
        break;
    case 0x08:
        r26 = 'B';
        break;
    case 0x10:
        r26 = '1';
        break;
    case 0x20:
        r26 = '2';
        break;
    case 0x40:
        r26 = '3';
        break;
    case 0x80:
        r26 = 'A';
        break;
    }

    return r26;
}

int main(void)
{
    DDRB = 0xFF; // B as output
    DDRC = 0xF0; // PC0-3 input and PC4-PC7 output (for the 4x4 keypad)
    adc_init();
    TIMSK = (1 << TOIE1);
    TCCR1B = (1 << CS12) | (0 << CS11) | (1 << CS10);
    TCNT1 = 64755;
    sei();
    /* Replace with your application code */
    while (1)
    {
        int r24, r25, c1, c2;

        do
        {
            scan_keypad_rising_edge_sim(15, &r24, &r25);
            c1 = keypad_to_ascii_sim(r24, r25);
        } while (r24 == 0 && r25 == 0); // while there is no pressed key, continue to scan if sth is pressed
        do
        {
            scan_keypad_rising_edge_sim(15, &r24, &r25);
            c2 = keypad_to_ascii_sim(r24, r25);
        } while (r24 == 0 && r25 == 0); // continue scanning for the 2nd digit
        if (c1 == '7' && c2 == '1')
        {                 // if the digit were 7 and then 1
            password = 1; // correct password
            MSB_led = 1;
            PORTB = 0x80 | leds; // switch on the PB7 and the state of the leds for 4 seconds
            for (int i = 0; i < 190; i++)
            {                                                // for 4 seconds
                scan_keypad_rising_edge_sim(15, &r24, &r25); // while scanning also (required in the remote access program)
                _delay_ms(1);
            }
            PORTB = 0x00 | leds; // after the 4 seconds switch off the PB7
            MSB_led = 0;
            password = 0;
        }
        else
        { // if the digits were not 7 and 1
            for (int i = 0; i < 4; i++)
            { // switch on and off the leds 4 times
                MSB_led = 1;
                for (int i = 0; i < 35; i++)
                {                                                // for 0.5 sec each
                    scan_keypad_rising_edge_sim(15, &r24, &r25); // while scanning (remote access program)
                    _delay_ms(1);
                }
                MSB_led = 0;
                for (int i = 0; i < 35; i++)
                {
                    scan_keypad_rising_edge_sim(15, &r24, &r25);
                    _delay_ms(1);
                }
            }
        }
    }
}
