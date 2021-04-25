#include "flood.h"


/**
 * Neighbor
 */
struct {
    Address  neighbor;
    uint32_t timestamp;
} neighbor_table[MAX_NEIGHBOR];

static bool is_neighbor() {

}


/**
 * Reboardcasting
 */
#define MAX_PACKET_SIZE 60
#define MAX_RING_SIZE 5

static struct {
    Timer timer;

    struct {
        uint8_t message[MAX_PACKET_SIZE];
        uint8_t len;
    } buffer[MAX_RING_SIZE];
    uint16_t head;
    uint16_t tail;
} rebroadcast_data;  // Rebroadcast data

static void rebroadcast() {
    // RoutingMsg msg;
    // msg.SeqNo       = currentFloodSeqNo;
    // msg.hopCount    = hopCount;
    // radioRequestTx(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}


/**
 * On packet received
 */
static void on_receive(Address source, MessageType type, void * message, uint8_t len) {
    flood_header_st *hdr = (flood_header_st *)message;
    uint8_t *msg = &((uint8_t *)message)[sizeof(flood_header_st)];
    uint8_t msg_len = len - sizeof(flood_header_st);

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
        debug("It's NOT a FLOOD_MSG_TYPE!");
    }
}


/**
 * Send
 */
void flood_send() {

}


/**
 * Init
 */
void flood_init(void) {
    // Initial 'neighbor_table'
    uint8_t i;
    for (i = 0; i < MAX_NEIGHBOR; i++) {
        neighbor_table[i].neighbor = BROADCAST_ADDR;
        neighbor_table[i].timestamp = 0;
    }

    // Initial 'rebroadcast_data'
    timerCreate(&rebroadcast_data.timer);
    rebroadcast_data.head = 0;
    rebroadcast_data.tail = 0;

    // Setup radio system
    radioSetRxHandler(on_receive);
}
