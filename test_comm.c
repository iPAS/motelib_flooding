#include "all_headers.h"
#include "flood.h"


static Timer timerLed;
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
void on_timer_fired_led_off()
{
    ledSet(0, 0);
}

void on_button_pushed(ButtonStatus s)
{
    // Toggle LED#1 whenever the button is pressed
    if (s == BUTTON_PRESSED)
    {
        // ledToggle(0);
        // ledToggle(1);
        // ledToggle(2);
        ledSet(0, 1);
        timerStart(&timerLed, TIMER_ONESHOT, 500, on_timer_fired_led_off);

        static int i = 0;
        debug("%d", i++);

        flood_send_to(0, message, sizeof(message));
    }
    else
    {
        // ledSet(0, 0);
    }
}

void on_approach_sink(void *message, uint8_t len)
{
    char *str;
    str = malloc(len*2 + 3);  // 3 for [, ], and \0
    arr_to_show((uint8_t *)message, len, str);
    debug("New massage approached the target sink: %s", str);
    free(str);
}


/**
 * The beginning
 */
void boot()
{
    debug("Booting.. Addr:%d, PAN_ID:%d, CH:%d", getAddress(), getPanId(), getChannel());

    timerCreate(&timerLed);
    buttonSetHandler(on_button_pushed);

    srand(getAddress());  // Set random seed
    flood_init();
    flood_set_rx_handler(on_approach_sink);
}
