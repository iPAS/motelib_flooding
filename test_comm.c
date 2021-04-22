#include "flood.h"


void button(ButtonStatus s) {
    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED) ledToggle(1);
}


void boot() {
    buttonSetHandler(button);

    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());
    flood_init();
}
