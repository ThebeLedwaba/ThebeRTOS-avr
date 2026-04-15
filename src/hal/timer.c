#include <avr/io.h>
#include <avr/interrupt.h>
#include "kernel.h"

// The assembly context switcher
extern void os_context_switch(void);

void timer_init(void) {
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC, pre 64
    OCR1A = 249; // 1ms @ 16MHz
    TIMSK1 |= (1 << OCIE1A);
}

// We MUST use NAKED ISR because we handle context saving manually in context.S
ISR(TIMER1_COMPA_vect, ISR_NAKED) {
    asm volatile("jmp os_context_switch");
}
