#include <stdlib.h>
#include "system.h"
#include "led.h"
#include "timer.h"
#include "radio.h"

#define FLOOD_MSG_TYPE 0x01

Timer    delayTimer;
uint16_t currentSeq;
uint8_t  hopCount;
Address  parent;

typedef struct flood_msg
{
    uint16_t seqNo;
    uint8_t  hopCount;
} FloodMsg;

void rebroadcast()
{
    static FloodMsg msg;

    msg.seqNo = currentSeq;
    msg.hopCount = hopCount;
    send(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}

void receive(Address source, MessageType type, char *message, uint16_t len) 
{
    if (type == FLOOD_MSG_TYPE)
    {
        FloodMsg *flood = (FloodMsg*)message;
        if (flood->seqNo > currentSeq)
        {
            currentSeq = flood->seqNo;
            hopCount = flood->hopCount + 1;
	    
	    debug("Change parent from %d to %d", parent, source);

            parent = source;
            startTimer(delayTimer, ONESHOT, rand()%500, rebroadcast);
        }
    }
}

void boot() 
{
    currentSeq = 0;
    hopCount   = 255;
    parent     = BROADCAST_ADDR; // invalid parent

    srand(getAddress());
    delayTimer = createTimer();
    setReceiveHandler(receive);
}

