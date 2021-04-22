#include "flood.h"


/**
 * Reboardcasting on unknowning of route.
 */
static Timer rebroadcast_timer;

static void rebroadcast()  // TODO: reboardcast all of the received message
{
    // RoutingMsg msg;
    // msg.SeqNo       = currentFloodSeqNo;
    // msg.hopCount    = hopCount;
    // radioRequestTx(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}


/**
 * On packet received
 */
static void on_receive(Address source, MessageType type, void * message, uint8_t len) {
    flood_header_st * hdr = (flood_header_st *)message;

    if (type == FLOOD_MSG_TYPE) {
        // timerStop(&beatTimer);
        // timerStart(&beatTimer, TIMER_ONESHOT, WAIT_PARENT, &reset_besthop);

        // debug("Change seqNo from current %d to %d", currentFloodSeqNo, msg->SeqNo);

        // if (msg->hopCount < MAX_HOP)
        // {
        //     hopCount = msg->hopCount + 1;
        //     timerStart(&delayTimer, TIMER_ONESHOT, rand()%500, &rebroadcast);
        // }

        // debug("Report seqNo %d < current %d to node %d", msg->SeqNo, currentFloodSeqNo, source);
    }
    else {

    }
}


/**
 * Send
 */
void flood_send() {}


/**
 * Init
 */
void flood_init(void) {
    srand(getAddress());  // Set random seed
    radioSetRxHandler(on_receive);

    timerCreate(&rebroadcast_timer);
}
