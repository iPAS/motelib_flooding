#ifndef __FLOOD_ROUTING__
#define __FLOOD_ROUTING__


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


extern void flood_init(void);


#endif  // __FLOOD_ROUTING__
