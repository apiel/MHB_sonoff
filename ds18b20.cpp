#include "config.h"

#ifdef PIN_DS18B20

#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ds18b20/ds18b20.h>
#include <esp8266.h>

#include "web.h"
#include "thermostat.h"

float prev_temperature = 0;

void ds18b20Task(void *pvParameters)
{
    while(1) {
        float temperature = ds18b20_read_temperature(PIN_DS18B20, DS18B20_ANY);
        // float temperature = ds18b20_measure_and_read(PIN_DS18B20, DS18B20_ANY);
        if (temperature) {
            if (prev_temperature != temperature) {
                printf("-> temp: %.1f\n", temperature);
                web_ws_temperature_send(temperature);
                temperature_changed((int)temperature);
            }
            prev_temperature = temperature;
        } else {
            printf("Could not read data from sensor\n");
        }

        // Three second delay...
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

#endif
