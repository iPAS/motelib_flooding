#include "flood_routing.h"


static Timer     delayTimer;
static uint16_t  currentFloodSeqNo;
static uint8_t   hopCount;
static Address   parent;

static uint16_t  reportSeqNo;
static Address   head;

static Timer     beatTimer;
static uint8_t   bestHop;
static uint8_t   cddBestHop; // Candidate
static Address   cddParent;


/**
 * Reboardcasting on unknowning of route.
 */
static
void rebroadcast()  // TODO: reboardcast all of the received message
{
    RoutingMsg msg;
    msg.SeqNo       = currentFloodSeqNo;
    msg.hopCount    = hopCount;
    radioRequestTx(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}


/**
 * Send back to the node sent beforehand.
 */
static
void report_back()
{
    RoutingMsg msg;
    msg.SeqNo       = reportSeqNo;
    msg.hopCount    = hopCount;
    radioRequestTx(head, REPORT_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}


/**
 * Set the new besthop (on specific condition, e.g., lowest hop count)
 */
static
void set_besthop(Address source, uint8_t newhop)
{
    debug("Change parent from %d to %d", parent, source);
    parent  = source;
    bestHop = newhop;
    debug("New best hop %d", bestHop);
}


/**
 * Reset the besthop to
 */
static
void reset_besthop()
{
    // set_besthop(BROADCAST_ADDR, MAX_HOP);
    set_besthop(cddParent, cddBestHop);
}


/**
 * On packet received
 */
static
void on_receive(Address source, MessageType type, void *message, uint8_t len)
{
    RoutingMsg *msg = (RoutingMsg*)message;

    if (type == FLOOD_MSG_TYPE)
    {
        if (msg->SeqNo > currentFloodSeqNo)
        {
            // Shortest hop problem
            if (msg->hopCount < bestHop || parent == BROADCAST_ADDR)
            {
                timerStop(&beatTimer);
                set_besthop(source, msg->hopCount);
            }
            else
            if (source == parent)
            {
                timerStop(&beatTimer);
            }
            else
            {
                timerStop(&beatTimer);
                cddParent  = source;  // Keep the new source for a candidate parent.
                cddBestHop = msg->hopCount;  // It might be greater than or equal to the current bestHop.
                // Re-check that parent exist by changing to new parent.
                // If the current parent still exist, it will acknowledge this node eventually.
                timerStart(&beatTimer, TIMER_ONESHOT, WAIT_PARENT, &reset_besthop);
            }

            // Dead or alive seqNo problem
            debug("Change seqNo from current %d to %d", currentFloodSeqNo, msg->SeqNo);
            currentFloodSeqNo = msg->SeqNo;

            if (msg->hopCount < MAX_HOP)
            {
                hopCount = msg->hopCount + 1;
                timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &rebroadcast);
            }

        }
        else
        if (msg->SeqNo == currentFloodSeqNo)
        {
           debug("Duplicated seqNo %d from node %d, discard", msg->SeqNo, source);
        }
        else  // if (msg->SeqNo < currentFloodSeqNo)
        {
            // Dead or alive seqNo problem.
            // Tell head that this node has a greater seqNo.
            // Report it back, up until the first hop.
            debug("Report seqNo %d < current %d to node %d", msg->SeqNo, currentFloodSeqNo, source);
            head        = source;
            reportSeqNo = currentFloodSeqNo;
            hopCount    = MAX_HOP - msg->hopCount;  // Prevent out-of-path routing.
            timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &report_back);  // Start report
        }
    }
    else
    if (type == REPORT_MSG_TYPE)  // Report whether dead or alive seqNo problem
    {
        if (msg->hopCount   < MAX_HOP            &&
            msg->SeqNo      > currentFloodSeqNo  &&
            source != BROADCAST_ADDR             &&
            parent != BROADCAST_ADDR)
        {
            debug("Report forwarded from node %d to parent %d", source, parent);
            head        = parent;       // Next hop reported
            reportSeqNo = msg->SeqNo;   // Update with the new seqNo (greater)
            hopCount    = msg->hopCount + 1;
            timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &report_back); // Forward report until the first hop
        }
    }
}


/**
 * Send
 */
void flood_send()
{

}


/**
 * Receive
 */
void flood_receive()
{

}


/**
 * Init
 */
void flood_init(void)
{
    currentFloodSeqNo = 0;
    hopCount   = MAX_HOP;
    bestHop    = MAX_HOP;
    parent     = BROADCAST_ADDR;  // Use invalid parent, why ?

    srand(getAddress());  // Set random seed
    radioSetRxHandler(on_receive);

    timerCreate(&delayTimer);
    timerCreate(&beatTimer);
}
