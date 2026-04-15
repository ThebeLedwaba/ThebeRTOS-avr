#include "kernel.h"

void list_add(List_t *list, TCB_t *task) {
    task->next = NULL;
    if (list->head == NULL) {
        list->head = task;
        list->tail = task;
    } else {
        list->tail->next = task;
        list->tail = task;
    }
}

TCB_t* list_remove_head(List_t *list) {
    if (list->head == NULL) return NULL;
    
    TCB_t *task = list->head;
    list->head = task->next;
    
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    task->next = NULL;
    return task;
}

uint8_t list_is_empty(List_t *list) {
    return (list->head == NULL);
}
