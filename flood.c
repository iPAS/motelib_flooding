#include <stdlib.h>

#include <motelib/system.h>
#include <motelib/led.h>
#include <motelib/timer.h>
#include <motelib/radio.h>
#include <motelib/uart.h>
#include <pt/pt.h>

#include "flood_routing.h"


void boot()
{
    flood_init();
}
