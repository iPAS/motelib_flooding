#include "queue.h"


void q_init(queue_t *q)
{
    q->head = NULL;
    q->tail = NULL;
    q->len = 0;
}

void q_enqueue(queue_t *q, void *data, uint8_t len)
{
    if (q->head != NULL)
    {
        q->head->next = (linklist_t *)malloc(sizeof(linklist_t));
        q->head = q->head->next;
    }
    else
    {
        q->head = (linklist_t *)malloc(sizeof(linklist_t));
    }

    memcpy(q->head->data, data, len);
    q->head->len = len;
    q->head->next = NULL;
    q->len++;
}

uint8_t q_dequeue(queue_t *q, void *data, uint8_t maxlen)
{
    // if (q->)

    return maxlen;
}