#include "neighbor.h"


static neighbor_t neighbors[MAX_NEIGHBOR];


void neighbor_init()
{
    uint8_t i;
    for (i = 0; i < MAX_NEIGHBOR; i++)
    {
        neighbors[i].addr = BROADCAST_ADDR;
        neighbors[i].rssi = 0;
    }
}