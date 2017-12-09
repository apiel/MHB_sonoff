
#include <espressif/esp_common.h>
#include <esp8266.h>

#include "config.h"
#include "led.h"

void led_blink(unsigned int count, unsigned int delay)
{
    gpio_enable(PIN_LED, GPIO_OUTPUT);
    for(; count > 0; count--) {
        gpio_write(PIN_LED, 1);
        sdk_os_delay_us(delay * 1000);
        gpio_write(PIN_LED, 0);
        sdk_os_delay_us(delay * 1000);
    }
}
