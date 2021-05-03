#include "flood.h"


// static uint8_t   currSeqNo;     // Current

// static Timer     parentalChallengeTimer;

// static Address   parentNode;
// static uint8_t   bestHopCount;
// static uint8_t   cddBestHop;    // Candidate that will be selected after
// static Address   cddParent;     //   'parentalChallengeTimer' fired.

static on_rx_sink on_approach_sink;  // Handler called if being the last node in the route.


/**
 * Historical delivery management
 */
typedef struct
{
    bool available;
    uint32_t timestamp;

    RoutingHeader recvHdr;  // Received header.
    Address parent;         // Intermediate node who sent us. Our way back to the source.
    uint8_t currSeqNo;      // Current 'seqNo' corresponding with the 'originSource'.
} delivery_history_t;

static delivery_history_t delivery_history[MAX_HISTORY];


static
void hist_init()
{
    uint8_t i;
    for (i = 0; i < MAX_HISTORY; i++)
    {
        delivery_history[i].available = false;
        delivery_history[i].timestamp = 0;
        delivery_history[i].parent = BROADCAST_ADDR;
    }
}


static
delivery_history_t *hist_find(RoutingHeader *hdr)
{
    delivery_history_t *hist = delivery_history,
                       *free_hist = NULL,
                       *oldest_hist = NULL;
    uint32_t oldest_timestamp = 0,
             now = timerTicks();
    uint8_t i;

    // Find in the table
    for (i = 0; i < MAX_HISTORY; i++, hist++)
    {
        if (!hist->available)
        {
            free_hist = hist;  // Memorize a free slot.
        }
        else
        {
            // Have found!
            if (hist->recvHdr.originSource == hdr->originSource)
            {
                hist->timestamp = now;
                return hist;
            }

            // Not found, find the oldest
            if (hist->timestamp > now)
                hist->timestamp = 0;  // Reset if overflow

            if (hist->timestamp >= oldest_timestamp)
            {
                oldest_timestamp = hist->timestamp;
                oldest_hist = hist;
            }
        }
    }

    // In case of not found, select an available slot, or, the oldest.
    hist = (free_hist != NULL)? free_hist : oldest_hist;
    hist->available = true;
    hist->timestamp = now;
    memcpy(&hist->recvHdr, hdr, sizeof(hist->recvHdr));
    hist->currSeqNo = hdr->seqNo - 1;  // Believe in the sender
    hist->parent = BROADCAST_ADDR;  // Back to none again.
    return hist;
}


/**
 * Set the new besthop (on specific condition, e.g., lowest hop count)
 */
// static
// void set_besthop(Address source, uint8_t newhop)
// {
//     debug("Change parent from %d to %d", parentNode, source);
//     parentNode  = source;
//     bestHopCount = newhop;
//     debug("New best hop %d", bestHopCount);
// }


/**
 * Reset the besthop to
 */
// static
// void reset_besthop()
// {
//     // set_besthop(BROADCAST_ADDR, MAX_HOP);
//     set_besthop(cddParent, cddBestHop);
// }


/**
 * On packet received
 */
static
void on_receive(Address source, MessageType type, void *message, uint8_t len)
{
    RoutingHeader *hdr = (RoutingHeader*)message;
    delivery_history_t *hist = hist_find(hdr);  // Historical data based on 'originSource'

    // ------------------------------------------------------------------------
    if (type == FLOOD_MSG_TYPE)
    {
        // --------------------------------
        // Flood message received correctly
        // --------------------------------
        if (hdr->seqNo > hist->currSeqNo  ||
            (hdr->seqNo == 0 && hist->currSeqNo == 255)  // On overflow
            )
        {
/*
            // Shortest hop recognition
            if (hdr->hopCount < bestHopCount  ||    // Shorter hop count, or
                parentNode == BROADCAST_ADDR        //  never has parent.
                )
            {
                // timerStop(&parentalChallengeTimer);
                set_besthop(source, hdr->hopCount);
            }
            else
            if (source == parentNode)  // The parent appears.
            {
                // timerStop(&parentalChallengeTimer);
            }
            else  // Hop not better, but keep the new source in mind as candidate parent.
            {
                cddParent  = source;
                cddBestHop = hdr->hopCount;  // It might be greater than or equal to the current bestHopCount.
                // Within specified time, if no clue from the parent, the candidate occupies.
                // timerStop(&parentalChallengeTimer);
                // timerStart(&parentalChallengeTimer, TIMER_ONESHOT, WAIT_PARENT, &reset_besthop);
            }
*/
            // Update parent
            debug("Change parent from %d to %d", hist->parent, source);
            hist->parent = source;
            debug("New best hop %d", hdr->hopCount);

            // Update with the newest greater seqNo
            debug("Change seqNo from current %d to %d", hist->currSeqNo, hdr->seqNo);
            hist->currSeqNo = hdr->seqNo;

            // If we are the final node!! -- the sink, then
            if (hdr->finalSink == getAddress())
            {
                // -----------------------------------------------
                // The message finally approaches its destination.
                // -----------------------------------------------
                if (on_approach_sink != NULL)
                    on_approach_sink(message, len);
            }
            // If not, rebroadcast
            else
            if (hdr->hopCount < MAX_HOP)
            {
                // -----------
                // Rebroadcast
                // -----------
                RoutingHeader fwd_hdr;
                memcpy(&fwd_hdr, hdr, sizeof(fwd_hdr));
                fwd_hdr.hopCount++;
                cq_send(BROADCAST_ADDR, FLOOD_MSG_TYPE, &fwd_hdr, sizeof(fwd_hdr));
                // TODO: also forward data further than header.
            }

        }

        // -------------------------------------
        // Flood message is duplicated, discard!
        // -------------------------------------
        else
        if (hdr->seqNo == hist->currSeqNo)
        {
           debug("Duplicated seqNo %d from node %d, discard", hdr->seqNo, source);
        }

        // ---------------------------------------------------------------------------
        // Flood message's seqNo lacks to the whole network, report back to the origin
        //  along the route through the source node.
        // Maybe the origin source was disconnected or shut down for a while.
        // ---------------------------------------------------------------------------
        else  // if (hdr->seqNo < hist->currSeqNo)
        {
            // --------------------------------------
            // Report back to the sender -- 'source'.
            // --------------------------------------
            debug("Report seqNo %d < current %d to node %d", hdr->seqNo, hist->currSeqNo, source);

            // Tell the upper node -- current sender -- that this node has a greater seqNo.
            // Report it back, up until the first hop.
            RoutingHeader fwd_hdr;
            memcpy(&fwd_hdr, hdr, sizeof(fwd_hdr));
            fwd_hdr.seqNo = hist->currSeqNo;  // Report with a greater 'seqNo' of this node.
            fwd_hdr.hopCount = MAX_HOP - hdr->hopCount;  // Prevent out-of-path routing.
            cq_send(source, REPORT_MSG_TYPE, &fwd_hdr, sizeof(fwd_hdr));
        }
    }
    // ------------------------------------------------------------------------
    else
    if (type == REPORT_MSG_TYPE)
    {
        if (hdr->hopCount < MAX_HOP)
        {
            if (hdr->seqNo > hist->currSeqNo  ||
                (hdr->seqNo == 0 && hist->currSeqNo == 255)  // On overflow
                )
            {
                debug("Change seqNo from current %d to %d", hist->currSeqNo, hdr->seqNo);
                hist->currSeqNo = hdr->seqNo;  // Update to fix the late currSeqNo at this node

                if (hist->parent != BROADCAST_ADDR)  // Tell the others
                {
                    // -----------------------------------------
                    // Next hop to be reported is node's parent.
                    // -----------------------------------------
                    debug("Report seqNo %d forwarded from node %d to parent %d", hdr->seqNo, source, hist->parent);

                    RoutingHeader fwd_hdr;
                    memcpy(&fwd_hdr, hdr, sizeof(fwd_hdr));
                    fwd_hdr.seqNo = hist->currSeqNo;
                    fwd_hdr.hopCount++;
                    cq_send(hist->parent, REPORT_MSG_TYPE, &fwd_hdr, sizeof(fwd_hdr));
                }
            }
        }
    }

    // Save the header to history table
    memcpy(&hist->recvHdr, hdr, sizeof(hist->recvHdr));
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

    // currSeqNo = 0;
    // bestHopCount = MAX_HOP;
    // parentNode = BROADCAST_ADDR;  // 'parent' is none

    cq_init();  // Initial communication queue
    hist_init();  // Initial delivery history for memeorizing a received packet.

    on_approach_sink = NULL;

    // timerCreate(&parentalChallengeTimer);

    radioSetRxHandler(on_receive);
}
