#ifndef __FLOOD_H__
#define __FLOOD_H__


#include "all_headers.h"


#define FLOOD_MSG_TYPE  0x01
#define MAX_HOP         255

typedef struct {
    Address     origin;
    uint8_t     hop_count;
    uint32_t    timestamp;
} flood_header_st;


extern void flood_init(void);


#endif  // __FLOOD_H__
