#include "commqueue.h"


static queue_t cq;


/**
 * Sending task
 */



/**
 * Send with random delay
 */
bool cq_send(Address dst, MessageType type, void *msg, uint8_t len)
{
    TxTask task;
    task.dst = dst;
    task.type = type;
    task.msg = malloc(len);
    task.len = len;
    timerCreate(&task.delayTimer);

    return true;
}


/**
 * Init 
 */
void cq_init()
{
    q_init(&cq);
}
