
#ifndef __TIMER_H__
#define __TIMER_H__

#include "action.h"

void timer_task(void *pvParameters);
int add_timer(void (*callback)(void), int seconds);
int add_timer(Action * object, int action, int seconds);

#endif
