#include "all_headers.h"
#include "flood.h"


static uint8_t message[] = {0,1,2,3,4,5,6,7,8,9};

static void arr_to_show(uint8_t *arr, uint8_t arr_len, char *str)
{
    str += sprintf(str, "[");
    while (arr_len-- > 0)
        str += sprintf(str, "%d,", *arr++);
    str--;  // remove the last comma
    str += sprintf(str, "]");
}


/**
 * Callback functions
 */
void on_button_pushed(ButtonStatus s)
{
    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED)
    {
        // ledToggle(0);
        // ledToggle(1);
        // ledToggle(2);
        ledSet(0, 1);

        char buf[30];
        arr_to_show(message, sizeof(message), buf);
        debug(">>> %s", buf);
    }
    else
    {
        ledSet(0, 0);
    }
}

void on_approach_sink(void *message, uint8_t len)
{
    debug("New massage approached the target sink");
}


/**
 * The beginning
 */
void boot()
{
    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());

    buttonSetHandler(on_button_pushed);

    srand(getAddress());  // Set random seed
    flood_init();
    flood_set_rx_handler(on_approach_sink);
}
