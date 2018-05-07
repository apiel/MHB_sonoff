#include <espressif/esp_common.h>
#include <esp8266.h>

#include "config.h"
#include "log.h"
#include "web.h"
#include "relay.h"

Relay::Relay(int pin, const char * id)
{
    _id = id;
    _pin = pin;
    relay_off();
}

const char * Relay::get_id()
{
    return _id;
}

bool Relay::_can_update()
{
    if (sdk_system_get_time() < 1000000 // let's allow to switch the relay at init of mcu
     || sdk_system_get_time() - _lastUpdate > 1000000) { // 1 sec
        _lastUpdate = sdk_system_get_time();
        return true;
    }
    return false;
}

void Relay::relay_on()
{
    if (_status != RELAY_ON && _can_update()) {
        logInfo("relay on\n");
        gpio_enable(_pin, GPIO_OUTPUT);
        gpio_write(_pin, RELAY_ON);
        _status = RELAY_ON;
        web_ws_relay_send_status(this);
    }
}

void Relay::relay_off()
{
    if (_status != RELAY_OFF && _can_update()) {
        logInfo("relay off\n");
        gpio_enable(_pin, GPIO_OUTPUT);
        gpio_write(_pin, RELAY_OFF);
        _status = RELAY_OFF;
        web_ws_relay_send_status(this);
    }
}

void Relay::relay_toggle()
{
    if (_status != RELAY_ON) {
        relay_on();
    } else {
        relay_off();
    }
}

int Relay::relay_status()
{
    printf("-> relay status %d\n", _status);
    return _status;
}

void Relay::operator() (int key)
{
    if (key == ACTION_RELAY_ON) {
        relay_on();
    } else if (key == ACTION_RELAY_OFF) {
        relay_off();
    } else {
        relay_toggle();
    }
}

// need task for timeout

#ifdef PIN_RELAY
Relay Relay1 = Relay(PIN_RELAY, "relay/1");
#endif

#ifdef PIN_RELAY_2
Relay Relay2 = Relay(PIN_RELAY_2, "relay/2");
#endif

#ifdef PIN_RELAY_3
Relay Relay3 = Relay(PIN_RELAY_3, "relay/3");
#endif

#ifdef PIN_RELAY_4
Relay Relay4 = Relay(PIN_RELAY_4, "relay/4");
#endif
