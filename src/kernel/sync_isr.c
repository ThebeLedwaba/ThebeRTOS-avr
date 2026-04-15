#include "kernel.h"
#include <avr/interrupt.h>

void os_sem_init(semaphore_t *sem, uint8_t init_val) {
    sem->count = init_val;
}

// Safe to call from ISRs
void os_sem_post(semaphore_t *sem) {
    uint8_t sreg = SREG;
    cli();
    
    if (sem->count < 255) {
        sem->count++;
    }
    
    // Check if any tasks were waiting for this (simple version: wake all)
    // In a more advanced version, we'd have a waiting list per semaphore.
    for (uint8_t i = 0; i < task_count; i++) {
        if (tasks[i].state == TASK_STATE_WAITING) {
            // This is a naive wake-up; in a real RTOS, we'd only wake 
            // tasks specifically waiting on 'sem'.
            tasks[i].state = TASK_STATE_READY;
        }
    }
    
    SREG = sreg;
}

void os_sem_wait(semaphore_t *sem) {
    while (1) {
        cli();
        if (sem->count > 0) {
            sem->count--;
            sei();
            return;
        }
        
        current_task->state = TASK_STATE_WAITING;
        sei();
        os_yield();
    }
}
