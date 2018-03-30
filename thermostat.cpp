#include <espressif/esp_common.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "relay.h"
#include "EEPROM.h"

int tempLimit = 16;

void temperature_changed(int temperature)
{
    if (tempLimit > 0) {
        if (temperature < tempLimit) {
            relay_on();
        } else {
            relay_off();
        }
    }
}

void set_thermostat(int temperature)
{
    logInfo("Set thermostat\n");
    tempLimit = temperature;
    EEPROM.write(EEPROM_THERMOSTAT, tempLimit);
    EEPROM.commit();
}

void thermostat_init(void)
{
    tempLimit = EEPROM.read(EEPROM_THERMOSTAT);
    logDebug("eeprom tempLimit\n");
}
