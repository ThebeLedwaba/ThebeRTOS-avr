#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "kernel.h"

// Task stack buffers
uint8_t stack_prod[STACK_SIZE];
uint8_t stack_cons[STACK_SIZE];
uint8_t stack_mon[STACK_SIZE + 64];

// Message Queue setup
#define QUEUE_CAPACITY 5
uint16_t queue_storage[QUEUE_CAPACITY];
Queue_t sensor_queue;

// External HAL functions
extern void timer_init(void);
extern void uart_init(void);
extern void uart_print(const char* str);

// Producer Task: Generates simulated ADC data
void task_producer(void) {
    uint16_t simulated_adc = 0;
    while (1) {
        simulated_adc += 10;
        if (simulated_adc > 1024) simulated_adc = 0;
        
        // Send data to queue (blocks if queue is full)
        os_queue_send(&sensor_queue, &simulated_adc);
        
        char buffer[32];
        sprintf(buffer, "[PROD]: Sent %u\r\n", simulated_adc);
        uart_print(buffer);
        
        os_delay(1500); // Produce every 1.5s
    }
}

// Consumer Task: Processes and displays data
void task_consumer(void) {
    uint16_t received_val;
    while (1) {
        // Receive data (blocks if queue is empty)
        os_queue_receive(&sensor_queue, &received_val);
        
        char buffer[32];
        sprintf(buffer, "[CONS]: Processed %u\r\n", received_val);
        uart_print(buffer);
        
        // Process time simulation
        os_delay(500);
    }
}

// Health Monitor Task
void health_monitor_task(void) {
    while (1) {
        os_delay(5000);
        uart_print("\r\n--- KERNEL STATS ---\r\n");
        for (uint8_t i = 0; i < task_count; i++) {
            char b[64];
            sprintf(b, "%s: Prio %u, StackFree %u, State %d\r\n",
                    tasks[i].name, tasks[i].priority, 
                    os_get_stack_free(i), tasks[i].state);
            uart_print(b);
        }
        uart_print("--------------------\r\n");
    }
}

int main(void) {
    uart_init();
    timer_init();
    os_init();
    os_timer_init();

    // Initialize the Message Queue
    os_queue_init(&sensor_queue, (uint8_t *)queue_storage, sizeof(uint16_t), QUEUE_CAPACITY);

    // Create tasks
    os_task_create(task_producer, stack_prod, "Producer", 3);
    os_task_create(task_consumer, stack_cons, "Consumer", 2);
    os_task_create(health_monitor_task, stack_mon, "Monitor", 1);

    os_start();
    sei();

    while (1);
    return 0;
}
