
#include <espressif/esp_common.h>
#include <esp8266.h>
#include <task.hpp>

#include "config.h"
#include "led.h"

void task_led_blink(unsigned int count, unsigned int delay)
{
    task_led_blink(count, delay, delay);
}

void task_led_blink(unsigned int count, unsigned int delay1, unsigned int delay2)
{
    gpio_enable(PIN_LED, GPIO_OUTPUT);
    for(; count > 0; count--) {
        gpio_write(PIN_LED, 0);
        vTaskDelay(delay1);
        gpio_write(PIN_LED, 1);
        vTaskDelay(delay2);
    }
}
