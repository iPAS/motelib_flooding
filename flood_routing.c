#include "flood_routing.h"


static Timer     delayTimer;
static uint16_t  currentFloodSeqNo;
static uint8_t   hopCount;
static Address   parent;

static uint16_t  reportSeqNo;
static Address   who_sent;

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
    FloodMsg msg;

    msg.floodSeqNo = currentFloodSeqNo;
    msg.hopCount   = hopCount;
    radioRequestTx(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}


/**
 * Send back to the node sent beforehand.
 */
static
void report_back()
{
    FloodMsg msg;

    msg.floodSeqNo = reportSeqNo;
    msg.hopCount   = hopCount;
    radioRequestTx(who_sent, REPORT_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
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
    FloodMsg *msg = (FloodMsg*)message;

    if (type == FLOOD_MSG_TYPE)
    {
        if (msg->floodSeqNo > currentFloodSeqNo)
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
            debug("Change seqNo from %d to %d", currentFloodSeqNo, msg->floodSeqNo);
            currentFloodSeqNo = msg->floodSeqNo;

            if (msg->hopCount < MAX_HOP)
            {
                hopCount = msg->hopCount + 1;
                timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &rebroadcast);
            }

        }
        else
        if (msg->floodSeqNo == currentFloodSeqNo)
        {
           debug("Duplicated seqNo %d from %d, discard", msg->floodSeqNo, source);
        }
        else
        {
            // Dead or alive seqNo problem
            debug("Report seqNo %d < %d to %d", msg->floodSeqNo, currentFloodSeqNo, source);
            who_sent    = source;
            reportSeqNo = currentFloodSeqNo;
            hopCount    = MAX_HOP - msg->hopCount;
            timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &report_back); // Start report
        }
    }
    else
    if (type == REPORT_MSG_TYPE)  // Report whether dead or alive seqNo problem
    {
        if (msg->hopCount    < MAX_HOP            &&
            msg->floodSeqNo  > currentFloodSeqNo  &&
            source != BROADCAST_ADDR              &&
            parent != BROADCAST_ADDR)
        {
            debug("Report forward from %d to %d", source, parent);

            who_sent    = parent;
            reportSeqNo = msg->floodSeqNo;
            hopCount    = msg->hopCount + 1;
            timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &report_back); // Forward report
        }
    }
}


/**
 * Init
 */
void flood_init(void)
{
    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());

    currentFloodSeqNo = 0;
    hopCount   = MAX_HOP;
    bestHop    = MAX_HOP;
    parent     = BROADCAST_ADDR; // Use invalid parent, why ?

    srand(getAddress());  // Set random seed
    radioSetRxHandler(on_receive);

    timerCreate(&delayTimer);
    timerCreate(&beatTimer);
}
