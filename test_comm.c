#include "flood.h"


void button(ButtonStatus s) {
    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED) ledToggle(1);
}


void boot() {
    srand(getAddress());  // Set random seed
    buttonSetHandler(button);

    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());
    flood_init();
}
