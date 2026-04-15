#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define MAX_TASKS 4
#define STACK_SIZE 64
#define STACK_FILL_PATTERN 0xAA
#define NUM_TIMERS 4

typedef enum {
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_WAITING
} task_state_t;

typedef struct TCB_s {
    uint8_t *stack_ptr;
    uint8_t *stack_base;
    task_state_t state;
    uint16_t sleep_ticks;
    uint8_t id;
    uint8_t priority;
    uint8_t base_priority;
    char name[12];
    struct TCB_s *next;
} TCB_t;

// Software Timer Structure
typedef enum { TIMER_ONE_SHOT, TIMER_PERIODIC } timer_type_t;
typedef struct {
    uint32_t period_ticks;
    uint32_t remaining_ticks;
    void (*callback)(void*);
    void *callback_arg;
    timer_type_t type;
    uint8_t active;
} SoftwareTimer_t;

// Message Queue Structure
typedef struct {
    uint8_t *buffer;
    uint8_t item_size;
    uint8_t capacity;
    volatile uint8_t count;
    volatile uint8_t head;
    volatile uint8_t tail;
    List_t tasks_waiting_to_send;
    List_t tasks_waiting_to_receive;
} Queue_t;

// List Structure
typedef struct {
    TCB_t *head;
    TCB_t *tail;
} List_t;

// Mutex Structure
typedef struct {
    uint8_t locked;
    TCB_t *owner;
    uint8_t owner_original_priority;
    List_t waiting_queue;
} Mutex_t;

// Binary Semaphore
typedef struct {
    volatile uint8_t count;
} semaphore_t;

// Kernel APIs
void os_init(void);
int8_t os_task_create(void (*task_func)(void), uint8_t *stack_buf, const char *name, uint8_t priority);
void os_start(void);
void os_schedule(void);
void os_yield(void);
void os_delay(uint16_t ms);

// Queue APIs
void os_queue_init(Queue_t *q, uint8_t *buffer, uint8_t item_size, uint8_t capacity);
void os_queue_send(Queue_t *q, const void *data);
void os_queue_receive(Queue_t *q, void *data);

// Timer APIs
void os_timer_init(void);
int8_t os_timer_create(uint32_t period, timer_type_t type, void (*callback)(void*), void *arg);
void os_timer_start(uint8_t timer_id);
void os_timer_stop(uint8_t timer_id);
void os_timer_process(void);

// Monitoring APIs
uint16_t os_get_stack_free(uint8_t task_id);
uint16_t get_free_heap(void);

// Sync APIs
void os_mutex_init(Mutex_t *mutex);
void os_mutex_lock(Mutex_t *mutex);
void os_mutex_unlock(Mutex_t *mutex);
void os_sem_init(semaphore_t *sem, uint8_t init_val);
void os_sem_post(semaphore_t *sem);
void os_sem_wait(semaphore_t *sem);

// List APIs
void list_add(List_t *list, TCB_t *task);
TCB_t* list_remove_head(List_t *list);
uint8_t list_is_empty(List_t *list);

// Global State
extern volatile TCB_t tasks[MAX_TASKS];
extern volatile TCB_t *current_task;
extern volatile uint8_t task_count;
extern volatile int8_t current_task_id;
extern volatile int8_t next_task_id;

#endif // KERNEL_H
