#include "kernel.h"
#include <avr/interrupt.h>
#include <string.h>

SoftwareTimer_t timers[NUM_TIMERS];
uint8_t timer_count = 0;

void os_timer_init(void) {
    memset(timers, 0, sizeof(timers));
    timer_count = 0;
}

int8_t os_timer_create(uint32_t period, timer_type_t type, void (*callback)(void*), void *arg) {
    if (timer_count >= NUM_TIMERS) return -1;
    
    SoftwareTimer_t *t = &timers[timer_count];
    t->period_ticks = period;
    t->remaining_ticks = period;
    t->callback = callback;
    t->callback_arg = arg;
    t->type = type;
    t->active = 0;
    
    return timer_count++;
}

void os_timer_start(uint8_t timer_id) {
    if (timer_id < NUM_TIMERS) {
        cli();
        timers[timer_id].remaining_ticks = timers[timer_id].period_ticks;
        timers[timer_id].active = 1;
        sei();
    }
}

void os_timer_stop(uint8_t timer_id) {
    if (timer_id < NUM_TIMERS) {
        timers[timer_id].active = 0;
    }
}

// Process timers (Called from os_tick in ISR context)
void os_timer_process(void) {
    for (uint8_t i = 0; i < timer_count; i++) {
        if (timers[i].active && timers[i].remaining_ticks > 0) {
            timers[i].remaining_ticks--;
            
            if (timers[i].remaining_ticks == 0) {
                // Execute callback
                if (timers[i].callback) {
                    timers[i].callback(timers[i].callback_arg);
                }
                
                if (timers[i].type == TIMER_PERIODIC) {
                    timers[i].remaining_ticks = timers[i].period_ticks;
                } else {
                    timers[i].active = 0;
                }
            }
        }
    }
}
