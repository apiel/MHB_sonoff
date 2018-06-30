#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <ssid_config.h>

// #include <espressif/esp_sta.h>
// #include <espressif/esp_wifi.h>

#include <semphr.h>

extern "C" {
  #include <paho_mqtt_c/MQTTESP8266.h>
  #include <paho_mqtt_c/MQTTClient.h>
}

#include "wifi.h"
#include "controller.h"

/* You can use http://test.mosquitto.org/ to test mqtt_client instead
 * of setting up your own MQTT server */
#define MQTT_HOST ("vps.alexparadise.com")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL

// SemaphoreHandle_t wifi_alive;
QueueHandle_t publish_queue;
#define PUB_MSG_LEN 16

char topic[22];

void  beat_task(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    char msg[PUB_MSG_LEN];
    int count = 0;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, 10000 / portTICK_PERIOD_MS);
        printf("beat\r\n");
        snprintf(msg, PUB_MSG_LEN, "Beat %d\r\n", count++);
        if (xQueueSend(publish_queue, (void *)msg, 0) == pdFALSE) {
            printf("Publish queue overflow.\r\n");
        }
    }
}

void  topic_received(mqtt_message_data_t *md)
{
    char msg[1048];
    memcpy(msg, md->message->payload, md->message->payloadlen);
    msg[md->message->payloadlen] = '\0';

    md->topic->lenstring.data[md->topic->lenstring.len] = '\0';

    char * action = md->topic->lenstring.data;
    action += strlen(topic) - 1;
    // printf("action: %s\nMsg: %s\n\n", action, msg);
    char tmp[1048]; // to remove later
    // controller_parse(action, md->message->payload);
    sprintf(tmp, "%s %s", action, msg);
    // printf("yoyoyo: %s\n", tmp);
    controller_parse(tmp);
}

int queue_publisher(mqtt_client_t * client)
{
    int ret = 0;
    while(1){

        char msg[PUB_MSG_LEN - 1] = "\0";
        while(xQueueReceive(publish_queue, (void *)msg, 0) ==
                pdTRUE){
            printf("got message to publish\r\n");
            mqtt_message_t message;
            message.payload = msg;
            message.payloadlen = PUB_MSG_LEN;
            message.dup = 0;
            message.qos = MQTT_QOS1;
            message.retained = 0;
            ret = mqtt_publish(client, "/beat", &message);
            if (ret != MQTT_SUCCESS ){
                printf("error while publishing message: %d\n", ret );
                break;
            }
        }

        ret = mqtt_yield(client, 1000);
        if (ret == MQTT_DISCONNECTED)
            break;
    }
    return ret;
}

void  mqtt_task(void *pvParameters)
{
    int ret         = 0;
    struct mqtt_network network;
    mqtt_client_t client   = mqtt_client_default;
    char mqtt_client_id[20];
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    mqtt_network_new( &network );
    memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
    strcpy(mqtt_client_id, get_uid());
    strcpy(topic, mqtt_client_id);
    strcat(topic, "/#");

    while(1) {
        // xSemaphoreTake(wifi_alive, portMAX_DELAY);
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ",__func__,
               MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if( ret ){
            printf("error: %d\n\r", ret);
            taskYIELD();
            vTaskDelay(2000);
            taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100,
                      mqtt_readbuf, 100);

        data.willFlag       = 0;
        data.MQTTVersion    = 3;
        data.clientID.cstring   = mqtt_client_id;
        data.username.cstring   = MQTT_USER;
        data.password.cstring   = MQTT_PASS;
        data.keepAliveInterval  = 10;
        data.cleansession   = 0;
        printf("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if(ret){
            printf("error: %d\n\r", ret);
            mqtt_network_disconnect(&network);
            taskYIELD();
            continue;
        }
        printf("done\r\n");
        mqtt_subscribe(&client, topic, MQTT_QOS1, topic_received);
        xQueueReset(publish_queue);

        ret = queue_publisher(&client);

        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        taskYIELD();
    }
}

void mqtt_start()
{
    publish_queue = xQueueCreate(3, PUB_MSG_LEN);
    // xTaskCreate(&beat_task, "beat_task", 256, NULL, 3, NULL);
    xTaskCreate(&mqtt_task, "mqtt_task", 1024, NULL, 4, NULL);
}
