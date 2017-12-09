#include "config.h"

#include "espressif/esp_common.h"
#include <esp8266.h>

#include "button.h"

void (*bt_callback)(void);

unsigned long startTime = 0; 

void handleButton(unsigned char pin) {
    int value = gpio_read(pin);
    // printf("handleButton pin %d value %d\n", pin, value); 

    if (value == 0) { // By default pin value is to 1
        startTime = sdk_system_get_time() / 1000000;
    } else if (startTime) { // when we push the button, it get connected to GND and became 0
        unsigned long duration = (sdk_system_get_time() / 1000000) - startTime;
        startTime = 0;
        printf("Button was press %lu second.\n", duration);
        if (duration > 5) {
            bt_callback();
        }
    }
}

void Button::init() {
    printf("Init button on pin %d\n", PIN_BUTTON);

    gpio_enable(PIN_BUTTON, GPIO_INPUT);
    gpio_set_interrupt(PIN_BUTTON, GPIO_INTTYPE_EDGE_ANY, handleButton);
}

Button::Button(void (*callback)(void)) {
    bt_callback = callback;
}