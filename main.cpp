#include "task.hpp"
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include "config.h"
#include "wifi.h"
#include "button.h"
#include "httpd.h"
#include "EEPROM.h"
#include "mqtt.h"

class task_rf_t: public esp_open_rtos::thread::task_t
{
private:
    void task()
    {
        printf("start rf task\n");

        while(true) {  
            vTaskDelay(10000);
        }      
    }    
};

task_rf_t task_rf;

extern "C" void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version: %s\n", sdk_system_get_sdk_version());
    printf("MyHomeBridge sonoff version: %s\n", VERSION);

    EEPROM.begin(EEPROM_SIZE);

    wifi_init();

    Button button = Button(wifi_toggle);
    button.init();

    task_rf.task_create("task_rf");

    // need to convert to cpp

    #ifdef MQTT_PORT
    xTaskCreate(&mqtt_task, "mqtt_task", 1024, NULL, 4, NULL);  
    #endif

    #ifdef HTTPD_PORT
    xTaskCreate(&httpd_task, "http_server", 1024, NULL, 2, NULL);
    #endif    
}
