#ifndef __BIND_H__
#define __BIND_H__


#include "all_headers.h"

typedef Timer zTimer;

extern void zTimerCreate(zTimer *timer);
extern void zTimerStart(zTimer *timer, TimerType type, uint16_t interval, TimerFired timerFired);
extern void zTimerStop(zTimer *timer);
extern uint16_t zTimerTicks();


#endif  // __BIND_H__
