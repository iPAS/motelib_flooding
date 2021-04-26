#include "flood.h"


static Timer     delayTxTimer;
static uint16_t  currSeqNo;     // Current
static uint8_t   currHopCount;

static uint16_t  reportSeqNo;   // report_back() argument
static Address   upperNode;     // report_back() argument

static Timer     parentalChallengeTimer;
static Address   parentNode;
static uint8_t   bestHopCount;
static uint8_t   cddBestHop;    // Candidate that will be selected after
static Address   cddParent;     //   'parentalChallengeTimer' fired.


/**
 * Reboardcasting on unknowning of route.
 */
static
void rebroadcast()  // TODO: reboardcast all of the received message, not just header
{
    RoutingHeader hdr;
    hdr.seqNo = currSeqNo;
    hdr.hopCount = currHopCount;
    radioRequestTx(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&hdr, sizeof(hdr), NULL);
}


/**
 * Send back to the node sent beforehand to fix the smaller seqNo
 */
static
void report_back()
{
    RoutingHeader hdr;
    hdr.seqNo = reportSeqNo;
    hdr.hopCount = currHopCount;
    radioRequestTx(upperNode, FIXSEQ_MSG_TYPE, (char*)&hdr, sizeof(hdr), NULL);
}


/**
 * Set the new besthop (on specific condition, e.g., lowest hop count)
 */
static
void set_besthop(Address source, uint8_t newhop)
{
    debug("Change parent from %d to %d", parentNode, source);
    parentNode  = source;
    bestHopCount = newhop;
    debug("New best hop %d", bestHopCount);
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
    RoutingHeader *hdr = (RoutingHeader*)message;

    // ------------------------------------------------------------------------
    if (type == FLOOD_MSG_TYPE)
    {
        // --------------------------------
        // Flood message received correctly
        // --------------------------------
        if (hdr->seqNo > currSeqNo)
        {
            // Shortest hop recognition
            if (hdr->hopCount < bestHopCount  ||    // Shorter hop count, or
                parentNode == BROADCAST_ADDR        //  never has parent.
                )
            {
                timerStop(&parentalChallengeTimer);
                set_besthop(source, hdr->hopCount);
            }
            else
            if (source == parentNode)
            {
                timerStop(&parentalChallengeTimer);
            }
            else
            {
                timerStop(&parentalChallengeTimer);
                cddParent  = source;  // Keep the new source for a candidate parent.
                cddBestHop = hdr->hopCount;  // It might be greater than or equal to the current bestHopCount.
                // Re-check that parent exist by changing to new parent.
                // If the current parent still exist, it will acknowledge this node eventually.
                timerStart(&parentalChallengeTimer, TIMER_ONESHOT, WAIT_PARENT, &reset_besthop);
            }

            // Dead or alive seqNo problem
            debug("Change seqNo from current %d to %d", currSeqNo, hdr->seqNo);
            currSeqNo = hdr->seqNo;

            // TODO: if we are the final node, do ...

            if (hdr->hopCount < MAX_HOP)
            {
                currHopCount = hdr->hopCount + 1;
                timerStart(&delayTxTimer, TIMER_ONESHOT, rand()%500, &rebroadcast);
            }

        }

        // -------------------------------------
        // Flood message is duplicated, discard!
        // -------------------------------------
        else
        if (hdr->seqNo == currSeqNo)
        {
           debug("Duplicated seqNo %d from node %d, discard", hdr->seqNo, source);
        }

        // -------------------------------------------------
        // Flood message's seqNo lacks to the whole network,
        //  report back to the original.
        // Maybe the original source shut down for a while.
        // -------------------------------------------------
        else  // if (hdr->seqNo < currSeqNo)
        {
            // Tell upperNode that this node has a greater seqNo.
            // Report it back, up until the first hop.
            debug("Report seqNo %d < current %d to node %d", hdr->seqNo, currSeqNo, source);
            upperNode = source;
            reportSeqNo = currSeqNo;  // Use our greater seqNo instead
            currHopCount = MAX_HOP - hdr->hopCount;  // Prevent out-of-path routing.
            timerStart(&delayTxTimer, TIMER_ONESHOT, rand()%500, &report_back);  // Start report
        }
    }
    // ------------------------------------------------------------------------
    else
    if (type == FIXSEQ_MSG_TYPE)  // Fix 
    {
        if (hdr->seqNo      > currSeqNo         &&
            hdr->hopCount   < MAX_HOP           &&
            source         != BROADCAST_ADDR    &&
            parentNode     != BROADCAST_ADDR)
        {
            debug("Report forwarded from node %d to parent %d", source, parentNode);
            upperNode = parentNode;  // Next hop reported
            reportSeqNo = hdr->seqNo;  // Update with the new seqNo (greater)
            currHopCount = hdr->hopCount + 1;
            timerStart(&delayTxTimer, TIMER_ONESHOT, rand()%500, &report_back); // Forward report until the first hop
        }
    }
}


/**
 * Init
 */
void flood_init(void)
{
    srand(getAddress());  // Set random seed

    currSeqNo = 0;
    currHopCount = MAX_HOP;
    bestHopCount = MAX_HOP;
    parentNode = BROADCAST_ADDR;  // parent is none

    timerCreate(&delayTxTimer);
    timerCreate(&parentalChallengeTimer);

    radioSetRxHandler(on_receive);
}
