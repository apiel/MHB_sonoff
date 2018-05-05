
#ifndef __TIMER_H__
#define __TIMER_H__

void timer_task(void *pvParameters);
int add_timer(void (*callback)(void), int seconds);
int add_timer(void * obj, void (*callback)(void *), int seconds);

#endif
