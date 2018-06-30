#include "config.h"

#include "espressif/esp_common.h"
#include <esp8266.h>

#include "button.h"
#include "log.h"

void (*bt_callback)(void);
void (*bt_callback_short)(void);

unsigned long startTime = 0; 

void handleButton(unsigned char pin) {
    int value = gpio_read(pin);
    if (value == 0) { // By default pin valu
        startTime = sdk_system_get_time() / 1000000;
    } else if (startTime) { // when we push the button, it get connected to GND and became 0
        unsigned long duration = (sdk_system_get_time() / 1000000) - startTime;
        startTime = 0;
        printf(" -> Button was press %lu second.\n", duration);
        if (duration > 3) {
            bt_callback();
        } else if (bt_callback_short) {
            bt_callback_short();
        }
    }
}

void Button::init() {
    logInfo("Init button\n");

    gpio_enable(PIN_BUTTON, GPIO_INPUT);
    gpio_set_interrupt(PIN_BUTTON, GPIO_INTTYPE_EDGE_ANY, handleButton);
}

Button::Button(void (*callback)(void)) {
    bt_callback = callback;
}

Button::Button(void (*callback)(void), void (*callback_short)(void)) {
    bt_callback = callback;
    bt_callback_short = callback_short;
}
