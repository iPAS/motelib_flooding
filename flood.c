#include "flood.h"
// TODO: redesign for the use of multi-source scenario.

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

static on_rx_sink on_approach_sink;  // Handler called if being the last node in the route.

static RoutingHeader latestHeader;


/**
 * Reboardcasting on unknowning of route.
 */
static
void rebroadcast()  // TODO: reboardcast all of the received message, not just header
{
    RoutingHeader hdr;
    memcpy(&hdr, &latestHeader, sizeof(hdr));
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
    memcpy(&hdr, &latestHeader, sizeof(hdr));
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
    memcpy(&latestHeader, hdr, sizeof(latestHeader));


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

            // Update with the newest greater seqNo
            debug("Change seqNo from current %d to %d", currSeqNo, hdr->seqNo);
            currSeqNo = hdr->seqNo;

            // If we are the final node!! -- the sink, then
            if (hdr->finalSink == getAddress())
            {
                if (on_approach_sink != NULL)
                    on_approach_sink(message, len);
            }
            // If not, rebroadcast
            else
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

        // ------------------------------------------------------------------------------
        // Flood message's seqNo lacks to the whole network, report back to the origin
        //  along the route through the source node.
        // Maybe the origin source was disconnected or shut down for a while.
        // ------------------------------------------------------------------------------
        else  // if (hdr->seqNo < currSeqNo)
        {
            // Tell upperNode that this node has a greater seqNo.
            // Report it back, up until the first hop.
            debug("Report seqNo %d < current %d to node %d", hdr->seqNo, currSeqNo, source);
            upperNode = source;                     // Send back to the sender
            currHopCount = MAX_HOP - hdr->hopCount; // Prevent out-of-path routing.
            reportSeqNo = currSeqNo;                // Report with the greater seqNo
            timerStart(&delayTxTimer, TIMER_ONESHOT, rand()%500, &report_back);  // Start report
        }
    }
    // ------------------------------------------------------------------------
    else
    if (type == FIXSEQ_MSG_TYPE)
    {
        if (hdr->hopCount < MAX_HOP)
        {
            if (hdr->seqNo > currSeqNo)
            {
                debug("Change seqNo from current %d to %d", currSeqNo, hdr->seqNo);
                currSeqNo = hdr->seqNo;  // Update to fix the late currSeqNo at this node

                if (parentNode != BROADCAST_ADDR)  // Tell the others
                {
                    debug("Report forwarded from node %d to parent %d", source, parentNode);
                    upperNode = parentNode;  // Next hop reported
                    currHopCount = hdr->hopCount + 1;
                    reportSeqNo = currSeqNo;
                    timerStart(&delayTxTimer, TIMER_ONESHOT, rand()%500, &report_back); // Forward report until the first hop
                }
            }
        }
    }
}


/**
 * Send & receive by flood routing
 */
void flood_set_rx_handler(on_rx_sink fn)
{
    on_approach_sink = fn;
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

    flood_set_rx_handler(NULL);

    timerCreate(&delayTxTimer);
    timerCreate(&parentalChallengeTimer);

    radioSetRxHandler(on_receive);
}
