#include "neighbor.h"


static neighbor_t neighbors[MAX_NEIGHBOR];
static on_nb_update on_update_info;


/**
 * Neighbor information management
 */
void neighbor_init()
{
    uint8_t i;
    for (i = 0; i < MAX_NEIGHBOR; i++)
    {
        neighbors[i].addr = BROADCAST_ADDR;
        neighbors[i].timestamp = 0;
        neighbors[i].rssi = 0;
    }

    on_update_info = NULL;
}


neighbor_t *neighbor_table()
{
    return neighbors;
}


neighbor_t *neighbor_find(Address addr)
{
    neighbor_t *nb = neighbors,
               *nb_free = NULL,
               *nb_oldest = NULL;
    uint32_t ts_oldest = 0,
             now = zTimerTicks();
    uint8_t i;
    for (i = 0; i < MAX_NEIGHBOR; i++, nb++)
    {
        if (nb->addr == BROADCAST_ADDR)
        {
            nb_free = nb;  // Memorize a free slot.
        }
        else
        {
            // Have found!
            if (nb->addr == addr)
            {
                nb->timestamp = now;
                return nb;
            }

            // Not found, find the oldest
            if (nb->timestamp > now)
                nb->timestamp = 0;  // Reset if overflow

            if (nb->timestamp >= ts_oldest)
            {
                ts_oldest = nb->timestamp;
                nb_oldest = nb;
            }
        }
    }

    // In case of not found, select an available slot, or, the oldest.
    nb = (nb_free != NULL)? nb_free : nb_oldest;
    nb->addr = addr;
    nb->timestamp = now;
    return nb;
}


/**
 * Update sending neighbor's information.
 */
void neighbor_update_info(Address source)
{
    neighbor_t *nb = neighbor_find(source);

    if (on_update_info != NULL)
    {
        (*on_update_info)(nb);
        return;
    }

    RadioRxStatus sts;
    radioGetRxStatus(&sts);
    nb->rssi = sts.rssi;
}
