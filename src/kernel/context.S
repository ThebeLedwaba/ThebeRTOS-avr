#include <avr/io.h>

#define SREG_ADDR _SFR_IO_ADDR(SREG)
#define SPL_ADDR  _SFR_IO_ADDR(SPL)
#define SPH_ADDR  _SFR_IO_ADDR(SPH)

.section .text

.extern tasks
.extern current_task
.extern current_task_id
.extern next_task_id
.extern os_schedule
.extern os_tick

/*
 * os_context_switch: The core switcher.
 */
.global os_context_switch
os_context_switch:
    /* 0. Update OS Ticks */
    call os_tick

    /* 1. Save all registers R0-R31 */
    push r0
    push r1
    /* ... and so on ... 
       Manually writing all 32 for clarity and correctness */
    push r2
    push r3
    push r4
    push r5
    push r6
    push r7
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    push r16
    push r17
    push r18
    push r19
    push r20
    push r21
    push r22
    push r23
    push r24
    push r25
    push r26
    push r27
    push r28
    push r29
    push r30
    push r31

    /* 2. Save SREG */
    in r0, SREG_ADDR
    push r0

    /* 3. Save current SP to current task's TCB */
    /* We need to get the pointer: tasks[current_task_id].stack_ptr */
    lds r24, current_task_id
    tst r24
    bmi skip_save           /* If current_task_id < 0, don't save */

    /* Each tcb_t is (uint8_ptr + uint8_status + uint8_id) = 4 bytes typically (on 8-bit ptrs it's 3 bytes, but let's check kernel.h)
       In kernel.h: tcb_t { uint8_t *stack_ptr; task_state_t state; uint8_t id; }
       On AVR, pointers are 2 bytes. So tcb_t is 2 + 1 + 1 = 4 bytes.
    */
    ldi r25, 0
    lsl r24                 /* r24 = id * 2 */
    lsl r24                 /* r24 = id * 4 */
    
    subi r24, lo8(-(tasks))
    sbci r25, hi8(-(tasks)) /* Z = tasks + (id * 4) */
    movw r30, r24           /* Z pointer now points to tasks[id] */

    in r0, SPL_ADDR
    st Z+, r0
    in r0, SPH_ADDR
    st Z, r0

skip_save:
    /* 4. Call scheduler to update next_task_id */
    call os_schedule

    /* 5. Load next task's SP */
    lds r24, next_task_id
    sts current_task_id, r24
    
    ldi r25, 0
    lsl r24
    lsl r24
    subi r24, lo8(-(tasks))
    sbci r25, hi8(-(tasks))
    movw r30, r24           /* Z points to tasks[next_task_id] */

    ld r0, Z+
    out SPL_ADDR, r0
    ld r0, Z
    out SPH_ADDR, r0

    /* 6. Restore SREG */
    pop r0
    out SREG_ADDR, r0

    /* 7. Restore all registers R31-R0 */
    pop r31
    pop r30
    pop r29
    pop r28
    pop r27
    pop r26
    pop r25
    pop r24
    pop r23
    pop r22
    pop r21
    pop r20
    pop r19
    pop r18
    pop r17
    pop r16
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop r7
    pop r6
    pop r5
    pop r4
    pop r3
    pop r2
    pop r1
    pop r0

    /* 8. Return (effectively to the new task's PC) */
    reti
