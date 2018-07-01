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

#include "config.h"
#include "wifi.h"
#include "controller.h"

QueueHandle_t publish_queue;

#define PAYLOAD_LEN 128

typedef struct
{
    char topic[64];
    char payload[PAYLOAD_LEN];
} mqtt_msg;

char subscribe_topic[22];

void mqtt_send(const char * topic, const char * payload)
{
    if (publish_queue) {
        // printf("Put mqtt msg in queue (%s): %s\n", topic, payload);
        mqtt_msg msg;
        strcpy(msg.topic, topic);
        strcpy(msg.payload, payload);
        if (xQueueSend(publish_queue, &msg, 0) == pdFALSE) {
            printf("Publish queue overflow (no more place in queue).\r\n");
        }
    }
}

void  topic_received(mqtt_message_data_t *md)
{
    char msg[PAYLOAD_LEN];
    memcpy(msg, md->message->payload, md->message->payloadlen);
    msg[md->message->payloadlen] = '\0';

    md->topic->lenstring.data[md->topic->lenstring.len] = '\0';

    char * action = md->topic->lenstring.data;
    action += strlen(subscribe_topic) - 1;
    // printf("action: %s\nMsg: %s\n\n", action, msg);
    char tmp[PAYLOAD_LEN]; // to remove later
    // controller_parse(action, md->message->payload);
    sprintf(tmp, "%s %s", action, msg);
    // printf("yoyoyo: %s\n", tmp);
    controller_parse(tmp);
}

int queue_publisher(mqtt_client_t * client)
{
    int ret = 0;
    while(1){
        mqtt_msg msg;
        while(xQueueReceive(publish_queue, &msg, 0) ==
                pdTRUE){
            // printf("got message to publish\r\n");
            mqtt_message_t message;
            message.payload = msg.payload;
            message.payloadlen = strlen(msg.payload);
            message.dup = 0;
            message.qos = MQTT_QOS1;
            message.retained = 0;
            ret = mqtt_publish(client, msg.topic, &message);
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
    strcpy(subscribe_topic, mqtt_client_id);
    strcat(subscribe_topic, "/#");

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
        mqtt_subscribe(&client, subscribe_topic, MQTT_QOS1, topic_received);
        xQueueReset(publish_queue);

        ret = queue_publisher(&client);

        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        taskYIELD();
    }
}

void mqtt_start()
{
    publish_queue = xQueueCreate(3, sizeof(mqtt_msg));
    xTaskCreate(&mqtt_task, "mqtt_task", 1024, NULL, 4, NULL);
}
