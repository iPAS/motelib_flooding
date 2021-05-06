#include "flood.h"


void button(ButtonStatus s)
{
    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED)
    {
        // ledToggle(0);
        // ledToggle(1);
        // ledToggle(2);
        ledSet(0, 1);
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


void boot()
{
    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());

    buttonSetHandler(button);

    srand(getAddress());  // Set random seed
    flood_init();
    flood_set_rx_handler(on_approach_sink);
}
