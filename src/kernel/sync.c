#include "kernel.h"
#include <avr/interrupt.h>

void os_mutex_init(Mutex_t *mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->owner_original_priority = 0;
    mutex->waiting_queue.head = NULL;
    mutex->waiting_queue.tail = NULL;
}

void os_mutex_lock(Mutex_t *mutex) {
    cli();
    
    if (!mutex->locked) {
        // Mutex is free, take it
        mutex->locked = 1;
        mutex->owner = (TCB_t *)current_task;
        mutex->owner_original_priority = current_task->priority;
        sei();
    } else {
        // Mutex is held by another task (m->owner)
        
        // Priority inheritance: if current task has higher priority than owner, 
        // boost owner's priority to match current task.
        if (current_task->priority > mutex->owner->priority) {
            mutex->owner->priority = current_task->priority;
        }

        current_task->state = TASK_STATE_WAITING;
        list_add(&mutex->waiting_queue, (TCB_t *)current_task);
        sei();
        
        os_yield();  // trigger context switch
    }
}

void os_mutex_unlock(Mutex_t *mutex) {
    cli();
    
    if (mutex->owner != (TCB_t *)current_task) {
        sei();
        return;
    }

    // Restore owner's original priority before releasing
    mutex->owner->priority = mutex->owner_original_priority;

    mutex->locked = 0;
    mutex->owner = NULL;

    if (!list_is_empty(&mutex->waiting_queue)) {
        TCB_t *next_task = list_remove_head(&mutex->waiting_queue);
        next_task->state = TASK_STATE_READY;
        
        // Handover mutex to next waiter
        mutex->locked = 1;
        mutex->owner = next_task;
        // The original priority for the NEW owner is its current one
        mutex->owner_original_priority = next_task->priority;
    }
    
    sei();
}
