#include <espressif/esp_common.h>
#include <esp8266.h>

#include "config.h"
#include "log.h"

void relay_on()
{
    logInfo("# relay on\n");
    gpio_enable(PIN_RELAY, GPIO_OUTPUT);
    gpio_write(PIN_RELAY, RELAY_ON);
}

void relay_off()
{
    logInfo("# relay off\n");
    gpio_enable(PIN_RELAY, GPIO_OUTPUT);
    gpio_write(PIN_RELAY, RELAY_OFF);
}

int relay_status()
{
    gpio_enable(PIN_RELAY, GPIO_INPUT);
    int status = gpio_read(PIN_RELAY);
    logInfo("# relay status %d\n", status);

    return status;
}
