#ifndef __FLOOD_H__
#define __FLOOD_H__


#include "all_headers.h"

#define FLOOD_MSG_TYPE  0x01
#define FIXSEQ_MSG_TYPE 0x22

#define MAX_HOP         255
#define WAIT_PARENT     10000

typedef struct
{
    uint8_t seqNo;
    uint8_t hopCount;
    Address originalSource;
    Address finalSink;
} RoutingHeader;


extern void flood_init(void);


#endif  // __FLOOD_H__
