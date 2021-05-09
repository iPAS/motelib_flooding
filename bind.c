#include "bind.h"


void zTimerCreate(zTimer *timer)
{
    timerCreate(timer);
}

void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, TimerFired timerFired)
{
    timerStart(timer, type, interval, timerFired);
}

void zTimerStop(Timer *timer)
{
    timerStop(timer);
}

uint16_t zTimerTicks()
{
    return timerTicks();
}
