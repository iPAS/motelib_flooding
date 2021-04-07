#include <stdlib.h>

#include <motelib/system.h>
#include <motelib/led.h>
#include <motelib/timer.h>
#include <motelib/radio.h>
#include <motelib/uart.h>
#include <pt/pt.h>

#define FLOOD_MSG_TYPE  0x01
#define REPORT_MSG_TYPE 0x22

#define MAX_HOP 255
#define WAIT_PARENT 10000

typedef struct flood_msg
{
    uint16_t seqNo;
    uint8_t  hopCount;
} FloodMsg;

Timer    delayTimer;
uint16_t currentSeq;
uint8_t  hopCount;
Address  parent;

uint16_t reportSeq;
Address  who_sent;

Timer    beatTimer;
uint8_t  bestHop;
uint8_t  cddBestHop; // Candidate
Address  cddParent;

void rebroadcast()
{
    FloodMsg msg;

    msg.seqNo    = currentSeq;
    msg.hopCount = hopCount;    
    // send(BROADCAST_ADDR, FLOOD_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}

void report_back()
{
    FloodMsg msg;
    
    msg.seqNo    = reportSeq;
    msg.hopCount = hopCount;
    // send(who_sent, REPORT_MSG_TYPE, (char*)&msg, sizeof(msg), NULL);
}

void set_besthop(Address source, uint8_t newhop)
{
    debug("Change parent from %d to %d", parent, source);
    parent  = source;    
    bestHop = newhop;
    debug("NEW best hop %d", bestHop);                         
}

void reset_besthop()
{
//    set_besthop(BROADCAST_ADDR, MAX_HOP);
    // set_besthop(cddParent, cddBestHop);
}

void receive(Address source, MessageType type, char *message, uint16_t len) 
{
    FloodMsg *flood = (FloodMsg*)message;   
    
    if (type == FLOOD_MSG_TYPE)
    {
        if (flood->seqNo > currentSeq)
        {        
            // Shortest hop problem
            if (flood->hopCount < bestHop || parent == BROADCAST_ADDR)
            {
                // stopTimer(beatTimer);
                // set_besthop(source, flood->hopCount);               
            }
            else
            if (source == parent)
            {
                // stopTimer(beatTimer);
            }
            else                
            {
                // stopTimer(beatTimer);
                cddParent  = source;
                cddBestHop = flood->hopCount;
                // startTimer(beatTimer, ONESHOT, WAIT_PARENT, &reset_besthop); // Re-check that parent exist
            }
            	                   
            // Dead or alive seqNo problem	                                   
            debug("Change sequence from %d to %d", currentSeq, flood->seqNo);
            currentSeq = flood->seqNo;
            
            if (flood->hopCount < MAX_HOP)
            {
                hopCount = flood->hopCount + 1; 
                // startTimer(delayTimer, ONESHOT, rand()%500, &rebroadcast);
            }
            
        }
        else
        if (flood->seqNo == currentSeq)
        {
//            debug("Dup SeqNO %d from %d", flood->seqNo, source);
        }
        else
        {   
            // Dead or alive seqNo problem
            debug("Report SeqNO %d < %d to %d", flood->seqNo, currentSeq, source);
            who_sent  = source;
            reportSeq = currentSeq;            
            hopCount  = MAX_HOP - flood->hopCount;
            // startTimer(delayTimer, ONESHOT, rand()%500, &report_back); // Start report
        }
    }
    else
    if (type == REPORT_MSG_TYPE) // Dead or alive seqNo problem
    {
        if (flood->hopCount < MAX_HOP       && 
            flood->seqNo    > currentSeq    && 
            source != BROADCAST_ADDR        &&
            parent != BROADCAST_ADDR)
        {
            debug("Report forward from %d to %d", source, parent);
        
            who_sent  = parent;   
            reportSeq = flood->seqNo;     
            hopCount  = flood->hopCount + 1;            
            // startTimer(delayTimer, ONESHOT, rand()%500, &report_back); // Forward report
        }
    }
}

void boot() 
{
    currentSeq = 0;
    hopCount   = MAX_HOP;    
    parent     = BROADCAST_ADDR; // Use invalid parent, why ?    

    srand(getAddress());
    // delayTimer = createTimer();
    // setReceiveHandler(receive);    
    
    bestHop    = MAX_HOP;
    // beatTimer  = createTimer();
}

