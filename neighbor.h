#ifndef __NEIGHBOR_H__
#define __NEIGHBOR_H__


#include "all_headers.h"
#include "flood.h"

#ifndef neighbor_t
typedef struct
{
    Address addr;
    uint32_t timestamp;

    uint8_t rssi;
    float snr;
} neighbor_t;
#endif

extern void neighbor_init();
extern neighbor_t *neighbor_find(Address addr);
extern neighbor_t *neighbor_table();
extern void neighbor_update_info(Address source);


#endif  // __NEIGHBOR_H__
