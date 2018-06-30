#ifndef _MQTT_h
#define _MQTT_h

#include "task.hpp"
#include "espressif/esp_common.h"

extern "C" {
  #include <paho_mqtt_c/MQTTESP8266.h>
  #include <paho_mqtt_c/MQTTClient.h>
}

class mqtt_t: public esp_open_rtos::thread::task_t
{
    private:
        void task();
        // int queue_publisher(mqtt_client_t * client);

    public:
        void init();
};

#endif
