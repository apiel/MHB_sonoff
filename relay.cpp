#include <espressif/esp_common.h>
#include <esp8266.h>

#include "config.h"
#include "log.h"
#include "web.h"

int status = -1;

void relay_on()
{
    if (status != RELAY_ON) {
        logInfo("# relay on\n");
        gpio_enable(PIN_RELAY, GPIO_OUTPUT);
        gpio_write(PIN_RELAY, RELAY_ON);
        status = RELAY_ON;
        web_ws_relay_send_status();
    }
}

void relay_off()
{
    if (status != RELAY_OFF) {
        logInfo("# relay off\n");
        gpio_enable(PIN_RELAY, GPIO_OUTPUT);
        gpio_write(PIN_RELAY, RELAY_OFF);
        status = RELAY_OFF;
        web_ws_relay_send_status();
    }
}

void relay_init()
{
    relay_off();
}

int relay_status()
{
    logInfo("# relay status %d\n", status);
    return status;
}

// need task for timeout
