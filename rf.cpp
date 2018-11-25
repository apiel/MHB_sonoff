#include <FreeRTOS.h>
#include <queue.h>
#include <string.h>
#include <task.hpp>
#include <esp/uart.h>

#include "rf.h"
#include "config.h"
#include "relay.h"
#include "timer.h"
#include "action.h"

Rf rf = Rf();

#ifdef RF_STORE
enum {
   numStore = sizeof(store) / sizeof(store[0])
};
#endif

void rf_task(void *pvParameters)
{
    char code[RF_CODE_SIZE];
    int pos = 0;
    while(1) {
        int c = uart_getc_nowait(0);
        if (c == '\n') {
            code[pos] = '\0';
            printf("RF Code from attiny %s\r\n", code);
            rf.consumer(code);
            pos = 0;
        } else {
            code[pos] = c;
            pos++;
        }
        taskYIELD();
        vTaskDelay(10);
    }
}

void Rf::consumer(char * result) {
#ifdef RF_STORE
    trigger(result);
#endif
}

#ifdef RF_STORE
void Rf::trigger_action_timer(Action * object, int action, int timer, int timer_id) {
    if (timer > 0) {
        // should we use xTimerCreate?
        add_timer(object, action, timer * timer, timer_id);
    } else {
        (* object)(action);
    }
}

Action * Rf::get_relay(int relay)
{
    Action * object;
    if (relay == PIN_RELAY_1) {
        object = &Relay1;
    }
#ifdef PIN_RELAY_2
    else if (relay == PIN_RELAY_2) {
        object = &Relay2;
    }
#endif
#ifdef PIN_RELAY_3
    else if (relay == PIN_RELAY_3) {
        object = &Relay3;
    }
#endif
#ifdef PIN_RELAY_4
    else if (relay == PIN_RELAY_4) {
        object = &Relay4;
    }
#endif
    else {
        object = &Relay1;
    }
    return object;
}

void Rf::trigger(char * code)
{
    for(int item = 0; item < numStore; item++) {
        printf("Compare code %s=%s ?\n", store[item].code, code);
        if (strcmp(store[item].code, code) == 0) {
            printf("yes\n");
            Action * object = get_relay(store[item].relay);
            trigger_action_timer(object, store[item].action, store[item].timer, store[item].timer_id);
            // break; // dont stop to get multiple event
        } else printf("no\n");
    }
}
#endif
