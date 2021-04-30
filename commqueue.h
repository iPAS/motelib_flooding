#ifndef __COMMQUEUE_H__
#define __COMMQUEUE_H__


#include "all_headers.h"
#include "queue.h"

typedef struct TxTask
{
    Address dst;
    MessageType type;

    void *msg;
    uint8_t len;

    Timer delayTimer;
} TxTask;

extern void cq_init();


#endif  // __COMMQUEUE_H__
