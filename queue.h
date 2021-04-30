#ifndef __QUEUE_H__
#define __QUEUE_H__


#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct linklist_t
{
    void *data;
    uint8_t len;
    struct linklist_t *next;
} linklist_t;

typedef struct
{
    linklist_t *tail;
    linklist_t *head;
    uint8_t len;
} queue_t;

extern void q_init(queue_t *q);
extern void q_enqueue(queue_t *q, void *data, uint8_t len);
extern uint8_t q_dequeue(queue_t *q, void *data, uint8_t maxlen);


#endif  // __QUEUE_H__
