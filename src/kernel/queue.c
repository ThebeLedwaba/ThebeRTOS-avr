#include "kernel.h"
#include <avr/interrupt.h>
#include <string.h>

void os_queue_init(Queue_t *q, uint8_t *buffer, uint8_t item_size, uint8_t capacity) {
    q->buffer = buffer;
    q->item_size = item_size;
    q->capacity = capacity;
    q->count = 0;
    q->head = 0;
    q->tail = 0;
    q->tasks_waiting_to_send.head = NULL;
    q->tasks_waiting_to_send.tail = NULL;
    q->tasks_waiting_to_receive.head = NULL;
    q->tasks_waiting_to_receive.tail = NULL;
}

void os_queue_send(Queue_t *q, const void *data) {
    cli();
    
    // If queue is full, block the sender
    while (q->count == q->capacity) {
        current_task->state = TASK_STATE_WAITING;
        list_add(&q->tasks_waiting_to_send, (TCB_t *)current_task);
        sei();
        os_yield();
        cli(); // Re-disable interrupts after waking up
    }

    // Copy data to tail
    memcpy(q->buffer + (q->tail * q->item_size), data, q->item_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;

    // Wake up a receiver if any are waiting
    if (!list_is_empty(&q->tasks_waiting_to_receive)) {
        TCB_t *receiver = list_remove_head(&q->tasks_waiting_to_receive);
        receiver->state = TASK_STATE_READY;
    }

    sei();
}

void os_queue_receive(Queue_t *q, void *data) {
    cli();
    
    // If queue is empty, block the receiver
    while (q->count == 0) {
        current_task->state = TASK_STATE_WAITING;
        list_add(&q->tasks_waiting_to_receive, (TCB_t *)current_task);
        sei();
        os_yield();
        cli(); // Re-disable interrupts after waking up
    }

    // Copy data from head
    memcpy(data, q->buffer + (q->head * q->item_size), q->item_size);
    q->head = (q->head + 1) % q->capacity;
    q->count--;

    // Wake up a sender if any are waiting
    if (!list_is_empty(&q->tasks_waiting_to_send)) {
        TCB_t *sender = list_remove_head(&q->tasks_waiting_to_send);
        sender->state = TASK_STATE_READY;
    }

    sei();
}
