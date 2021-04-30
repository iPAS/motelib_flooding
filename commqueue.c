#include "commqueue.h"


static TxTask *ll_head_TxTask, *ll_tail_TxTask;


/**
 * Send with random delay
 */
bool cq_send_random()
{

    return true;
}


/**
 * Init 
 */
void cq_init()
{
    ll_head_TxTask = ll_tail_TxTask = NULL;
}
