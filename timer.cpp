#ifndef TEST
#include <espressif/esp_common.h>
#else
#include "test/mock.cpp"
#endif

#include <string.h>

#define TIMER_SIZE 10

struct Timer {
    void (*callback)(void);
    unsigned long time = 0;
};
Timer timer[TIMER_SIZE];

int get_free_timer() {
    int pos = 0;
    unsigned long current_time = sdk_system_get_time();
    for(; pos < TIMER_SIZE && timer[pos].time > current_time; pos++);
    return timer[pos].time > current_time ? -1 : pos;
}

int add_timer(void (*callback)(void), int seconds)
{
    int pos = get_free_timer();
    if (pos > -1) {
        timer[pos].time = sdk_system_get_time() + (seconds * 1000000);
        timer[pos].callback = callback;
    }
    return pos;
}

void execute_timer()
{
    int pos = 0;
    unsigned long current_time = sdk_system_get_time();
    for(; pos < TIMER_SIZE; pos++) {
        if (timer[pos].time > 0 && timer[pos].time < current_time) {
            timer[pos].callback();
            timer[pos].time = 0;
        }
    }
}
