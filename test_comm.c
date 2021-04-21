#include "flood_routing.h"


void button(ButtonStatus s)
{
    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED)
        ledToggle(1);
}


void boot()
{
    buttonSetHandler(button);

    flood_init();
}
