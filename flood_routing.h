#ifndef __FLOOD_ROUTING__
#define __FLOOD_ROUTING__


#include "all_headers.h"

#define FLOOD_MSG_TYPE  0x01
#define REPORT_MSG_TYPE 0x22

#define MAX_HOP         255
#define WAIT_PARENT     10000

typedef struct
{
    uint8_t SeqNo;
    uint8_t hopCount;
} RoutingMsg;


extern void flood_init(void);


#endif  // __FLOOD_ROUTING__
