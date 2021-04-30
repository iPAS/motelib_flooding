#ifndef __COMMQUEUE_H__
#define __COMMQUEUE_H__


#include "all_headers.h"

typedef struct TxTask
{
    Timer delayTimer;

    Address dst;
    MessageType type;

    uint8_t *message;
    uint8_t len;

    struct TxTask *next;
} TxTask;

extern void cq_init();


#endif  // __COMMQUEUE_H__
