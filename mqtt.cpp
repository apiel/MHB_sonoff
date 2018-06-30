#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <FreeRTOS.h>
// #include <task.h>
#include <ssid_config.h>

// #include <espressif/esp_sta.h>
// #include <espressif/esp_wifi.h>

#include <semphr.h>

extern "C" {
  #include <paho_mqtt_c/MQTTESP8266.h>
  #include <paho_mqtt_c/MQTTClient.h>
}

#include "wifi.h"
#include "mqtt.h"

/* You can use http://test.mosquitto.org/ to test mqtt_client instead
 * of setting up your own MQTT server */
#define MQTT_HOST ("vps.alexparadise.com")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL

// SemaphoreHandle_t wifi_alive;
QueueHandle_t publish_queue;
#define PUB_MSG_LEN 16

void  topic_received(mqtt_message_data_t *md)
{
    int i;
    mqtt_message_t *message = md->message;
    printf("Received: ");
    for( i = 0; i < md->topic->lenstring.len; ++i)
        printf("%c", md->topic->lenstring.data[ i ]);

    printf(" = ");
    for( i = 0; i < (int)message->payloadlen; ++i)
        printf("%c", ((char *)(message->payload))[i]);

    printf("\r\n");
}

// int mqtt_t::queue_publisher(mqtt_client_t * client)
// {
//     int ret = 0;
//     while(1){

//         char msg[PUB_MSG_LEN - 1] = "\0";
//         while(xQueueReceive(publish_queue, (void *)msg, 0) ==
//                 pdTRUE){
//             printf("got message to publish\r\n");
//             mqtt_message_t message;
//             message.payload = msg;
//             message.payloadlen = PUB_MSG_LEN;
//             message.dup = 0;
//             message.qos = MQTT_QOS1;
//             message.retained = 0;
//             ret = mqtt_publish(client, "/beat", &message);
//             if (ret != MQTT_SUCCESS ){
//                 printf("error while publishing message: %d\n", ret );
//                 break;
//             }
//         }

//         ret = mqtt_yield(client, 1000);
//         if (ret == MQTT_DISCONNECTED)
//             break;
//     }
//     return ret;
// }

void  mqtt_t::task()
{
    int ret         = 0;
    struct mqtt_network network;
    mqtt_client_t client   = mqtt_client_default;
    char mqtt_client_id[20];
    char topic[22];
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
            // taskYIELD();
            // vTaskDelay(2000);
            sleep(1000);
            // taskYIELD();
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100,
                      mqtt_readbuf, 100);

        // data.willFlag       = 0;
        // data.MQTTVersion    = 3;
        // data.clientID.cstring   = mqtt_client_id;
        // data.username.cstring   = MQTT_USER;
        // data.password.cstring   = MQTT_PASS;
        // data.keepAliveInterval  = 10;
        // data.cleansession   = 0;
        // printf("Send MQTT connect ... ");
        // ret = mqtt_connect(&client, &data);
        // if(ret){
        //     printf("error: %d\n\r", ret);
        //     mqtt_network_disconnect(&network);
        //     // taskYIELD();
        //     continue;
        // }
        // printf("done\r\n");
        // mqtt_subscribe(&client, topic, MQTT_QOS1, topic_received);
        // xQueueReset(publish_queue);
        sleep(1000);

        // // ret = queue_publisher(&client);
        // while(1){

        //     char msg[PUB_MSG_LEN - 1] = "\0";
        //     while(xQueueReceive(publish_queue, (void *)msg, 0) ==
        //             pdTRUE){
        //         printf("got message to publish\r\n");
        //         mqtt_message_t message;
        //         message.payload = msg;
        //         message.payloadlen = PUB_MSG_LEN;
        //         message.dup = 0;
        //         message.qos = MQTT_QOS1;
        //         message.retained = 0;
        //         ret = mqtt_publish(&client, "/beat", &message);
        //         if (ret != MQTT_SUCCESS ){
        //             printf("error while publishing message: %d\n", ret );
        //             break;
        //         }
        //     }

        //     ret = mqtt_yield(&client, 1000);
        //     if (ret == MQTT_DISCONNECTED)
        //         break;
        // }

        // printf("Connection dropped, request restart\n\r");
        // mqtt_network_disconnect(&network);
        // // taskYIELD();
    }
}

void mqtt_t::init()
{
    publish_queue = xQueueCreate(3, PUB_MSG_LEN);
}
