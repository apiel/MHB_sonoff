#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <FreeRTOS.h>
#include <task.h>
#include <dht/dht.h>
#include <esp8266.h>

#include "dht.h"
#include "config.h"

const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;

void dhtTask(void *pvParameters)
{
    int16_t temperature = 0;
    int16_t humidity = 0;

    // DHT sensors that come mounted on a PCB generally have
    // pull-up resistors on the data pin.  It is recommended
    // to provide an external pull-up resistor otherwise...
    gpio_set_pullup(PIN_DHT, false, false);

    while(1) {
        if (dht_read_data(sensor_type, PIN_DHT, &humidity, &temperature)) {
            printf("Humidity: %d%% Temp: %dC\n",
                    humidity / 10,
                    temperature / 10); //dht11 precision to deciaml, no need to try to get coma
        } else {
            printf("Could not read data from sensor\n");
        }

        // Three second delay...
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}
