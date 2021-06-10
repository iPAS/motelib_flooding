#include "all_headers.h"
#include "neighbor.h"
#include "flood.h"


static zTimer timerLed;
static uint8_t message[] = {0,1,2,3,4,5,6,7,8,9};


static void show_arr(uint8_t *arr, uint8_t arr_len)
{
    const uint8_t LOT_SIZE = 16;
    char str_buf[80];
    char *str;

    uint16_t len;
    while (arr_len > 0)
    {
        for (len = 0, str = str_buf; len < arr_len && len < LOT_SIZE; len++)
        {
            str += sprintf(str, "%02X, ", *arr++);
        }
        arr_len -= len;
        debug("    %s", str_buf);
    }
}


/**
 * Send node's status to
 */
typedef struct __attribute__((packed))  // For sending through the network.
{
    Address addr;
    uint8_t rssi;   // Degree 0-255 of RSSI
} neighbor_status_t;


void on_neighbor_update(neighbor_t *nb)
{
    RadioRxStatus sts;
    radioGetRxStatus(&sts);
    nb->rssi = sts.rssi;
}


bool send_status_to(Address sink)
{
    neighbor_status_t status[MAX_NEIGHBOR];
    neighbor_status_t *sts = status;
    neighbor_t *nb = neighbor_table();
    uint8_t i, cnt;
    for (i = 0, cnt = 0; i < MAX_NEIGHBOR; i++, nb++)
    {
        if (nb->addr != BROADCAST_ADDR)
        {
            sts->addr = nb->addr;
            sts->rssi = nb->rssi;
            sts++;
            cnt++;
        }
    }
    return flood_send_to(sink, status, cnt*sizeof(neighbor_status_t));
}


/**
 * Callback functions
 */
void on_timer_fired_led_off()
{
    ledSet(0, 0);
}


void on_button_pushed(ButtonStatus s)
{
    static uint8_t count = 0;

    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED)
    {
        // ledToggle(0);
        // ledToggle(1);
        // ledToggle(2);
        ledSet(0, 1);
        zTimerStart(&timerLed, TIMER_ONESHOT, 500, on_timer_fired_led_off);


        if (count++ < 2)
            flood_send_to(0, message, sizeof(message));
        else
            send_status_to(0);
    }
    else
    {
        // ledSet(0, 0);
    }
}


void on_approach_sink(void *message, uint8_t len)
{
    // XXX: in system.c, MAX_CMD_LEN is only 255. So, cannot print over than the buf size.
    debug("New massage approached the finalSink:");
    show_arr(message, len);
}


/**
 * The beginning
 */
void boot()
{
    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());

    zTimerCreate(&timerLed);
    buttonSetHandler(on_button_pushed);

    srand(getAddress());  // Set random seed

    flood_init();
    flood_set_rx_handler(on_approach_sink);
    neighbor_set_update_handler(on_neighbor_update);
}
