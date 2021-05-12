#include "bind.h"


void zTimerCreate(zTimer *timer)
{
    timerCreate(timer);
}

void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, zTimerFired onFired)
{
    timerStart(timer, type, interval, onFired);
}

void zTimerStop(Timer *timer)
{
    timerStop(timer);
}

uint16_t zTimerTicks()
{
    return timerTicks();
}
