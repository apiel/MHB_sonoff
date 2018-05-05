#ifndef TEST
#include <espressif/esp_common.h>
#include <lwip/api.h> // vtask
#else
#include "test/mock.cpp"
#endif

#include <string.h>

#include "log.h"

#define TIMER_SIZE 10

// look at http://www.cplusplus.com/forum/general/136410/

struct Timer {
    void (*callback)(void);
    void (*callbackObject)(void *);
	void* object = NULL;
    unsigned long time = 0;
};
Timer timer[TIMER_SIZE];

int get_free_timer() {
    int pos = 0;
    unsigned long current_time = sdk_system_get_time();
    for(; pos < TIMER_SIZE && timer[pos].time > current_time; pos++);
    return timer[pos].time > current_time ? -1 : pos;
}

int add_timer(int seconds)
{
    int pos = get_free_timer();
    if (pos > -1) {
        timer[pos].time = sdk_system_get_time() + (seconds * 1000000);
    }
    logInfo("add timer\n");
    return pos;
}

int add_timer(void (*callback)(void), int seconds)
{
    int pos = add_timer(seconds);
    if (pos > -1) {
        timer[pos].callback = callback;
    }
    return pos;
}

int add_timer(void * obj, void (*callback)(void *), int seconds)
{
    int pos = add_timer(seconds);
    if (pos > -1) {
        timer[pos].callbackObject = callback;
        timer[pos].object = obj;
    }
    return pos;
}

void execute_timer()
{
    int pos = 0;
    unsigned long current_time = sdk_system_get_time();
    for(; pos < TIMER_SIZE; pos++) {
        if (timer[pos].time > 0 && timer[pos].time < current_time) {
            if (timer[pos].object != NULL) {
                timer[pos].callbackObject(timer[pos].object);
            } else {
                timer[pos].callback();
            }
            timer[pos].time = 0;
        }
    }
}

void timer_task(void *pvParameters)
{
    while(1) {
        vTaskDelay(100);
        execute_timer();
    }
}
