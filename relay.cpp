#include <espressif/esp_common.h>
#include <esp8266.h>

#include "config.h"
#include "log.h"

int value;

void relay_on()
{
    logInfo("# relay on\n");
    gpio_enable(PIN_RELAY, GPIO_OUTPUT);
    gpio_write(PIN_RELAY, RELAY_ON);
    value = RELAY_ON;
}

void relay_off()
{
    logInfo("# relay off\n");
    gpio_enable(PIN_RELAY, GPIO_OUTPUT);
    gpio_write(PIN_RELAY, RELAY_OFF);
    value = RELAY_OFF;
}

void relay_init()
{
    relay_off();
}

int relay_status()
{
    logInfo("# relay status %d\n", value);
    return value;
}
