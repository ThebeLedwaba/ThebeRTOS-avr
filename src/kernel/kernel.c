#include "kernel.h"
#include <avr/interrupt.h>
#include <string.h>
#include <stdlib.h>

volatile TCB_t tasks[MAX_TASKS];
volatile TCB_t *current_task = NULL;
volatile int8_t current_task_id = -1;
volatile int8_t next_task_id = -1;
volatile uint8_t task_count = 0;

extern char *__brkval;
extern char __heap_start;

void os_init(void) {
    memset((void*)tasks, 0, sizeof(tasks));
    task_count = 0;
    current_task_id = -1;
}

int8_t os_task_create(void (*task_func)(void), uint8_t *stack_buf, const char *name, uint8_t priority) {
    if (task_count >= MAX_TASKS) return -1;

    TCB_t *new_tcb = (TCB_t *)&tasks[task_count];
    memset(stack_buf, STACK_FILL_PATTERN, STACK_SIZE);

    uint8_t *stk = stack_buf + STACK_SIZE - 1;
    *stk-- = (uint8_t)((uint16_t)task_func & 0xFF);
    *stk-- = (uint8_t)(((uint16_t)task_func >> 8) & 0xFF);
    for (int i = 0; i < 32; i++) *stk-- = 0x00;
    *stk-- = 0x80;

    new_tcb->stack_ptr = stk;
    new_tcb->stack_base = stack_buf;
    new_tcb->state = TASK_STATE_READY;
    new_tcb->id = task_count;
    new_tcb->priority = priority;
    new_tcb->base_priority = priority;
    strncpy(new_tcb->name, name, 11);
    new_tcb->name[11] = '\0';

    return task_count++;
}

void os_tick(void) {
    for (uint8_t i = 0; i < task_count; i++) {
        if (tasks[i].sleep_ticks > 0) tasks[i].sleep_ticks--;
    }
    os_timer_process(); // Process software timers
}

void os_schedule(void) {
    if (task_count == 0) return;

    uint8_t max_priority = 0;
    int8_t high_task_id = -1;

    // 1. Find Max Priority among READY tasks
    for (uint8_t i = 0; i < task_count; i++) {
        if (tasks[i].state == TASK_STATE_READY && tasks[i].sleep_ticks == 0) {
            if (tasks[i].priority > max_priority) {
                max_priority = tasks[i].priority;
            }
        }
    }

    // 2. Find High Task ID (with same priority Round-Robin logic)
    int8_t search_id = (current_task_id + 1) % task_count;
    for (uint8_t i = 0; i < task_count; i++) {
        if (tasks[search_id].state == TASK_STATE_READY && 
            tasks[search_id].sleep_ticks == 0 && 
            tasks[search_id].priority == max_priority) {
            next_task_id = search_id;
            current_task = (TCB_t *)&tasks[next_task_id];
            return;
        }
        search_id = (search_id + 1) % task_count;
    }
}

uint16_t os_get_stack_free(uint8_t task_id) {
    if (task_id >= task_count) return 0;
    TCB_t *task = (TCB_t *)&tasks[task_id];
    uint16_t unused = 0;
    for (uint16_t i = 0; i < STACK_SIZE; i++) {
        if (task->stack_base[i] == STACK_FILL_PATTERN) unused++;
        else break;
    }
    return unused;
}

uint16_t get_free_heap(void) {
    char top;
    return (uint16_t)(&top - (__brkval == 0 ? &__heap_start : __brkval));
}

void os_delay(uint16_t ms) {
    cli();
    current_task->sleep_ticks = ms;
    sei();
    os_yield();
}

extern void os_context_switch(void);
void os_yield(void) {
    asm volatile("call os_context_switch");
}

void os_start(void) {
    if (task_count == 0) return;
    current_task_id = -1;
}
