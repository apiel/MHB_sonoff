#include <lwip/api.h>
#include <string.h>
#include <espressif/esp_common.h>

extern "C" {
    #include <paho_mqtt_c/MQTTESP8266.h>
    #include <paho_mqtt_c/MQTTClient.h>
}

#include "wifi.h"
#include "config.h"
#include "mqtt.h"

#define topics_size 24

mqtt_client_t client = mqtt_client_default;
char mqtt_client_id[20];

// char topics[topics_size][128];
// size_t topics_count = 0;

// bool is_topic_already_inserted(char * topic) {
//     int index = 0;
//     for (; index < topics_count; index++) {
//         size_t len = strlen(topics[index]);
//         if (topics[index][len-1] == '+') {
//             len--;
//             char topicCmp[128];
//             memcpy(topicCmp, topics[index], len);
//             topicCmp[len] = '\0';

//             char * found = strstr(topic, topicCmp);
//             if (found && (topic - found) == 0) {
//                 // printf("Topic to compare A: %s %s %d\n", topicCmp, found, topic - found);            
//                 return true;
//             }
//         } 
//         else {
//             printf("Topic to compare B: %s\n", topics[index]);
//         }
//     }

//     return false;
// }

// void insert_topic(char * topic) {
//     if (!is_topic_already_inserted(topic)) {
//         if (topics_count < topics_size) {
//             printf("Insert topic: %s\n", topic);
//             strcpy(topics[topics_count++], topic);   
//         } else {
//             printf("Topics cannot be inserted, limit of %d reached..\n", topics_size);
//         }
//     }
// }

// void mqtt_init() 
// {
//     memset(mqtt_client_id, 0, sizeof(mqtt_client_id));
//     strcpy(mqtt_client_id, get_uid());    

//     // we could define the topic size to share the same size between each .c file
//     char topic[128];
//     // let s use sprintf
//     strcpy(topic, MHB_USER);
//     strcat(topic, "/");
//     strcat(topic, MHB_ZONE);    
//     strcat(topic, "/");
//     strcat(topic, mqtt_client_id);
//     strcat(topic, "/+");
//     insert_topic(topic);

//     // let s use sprintf
//     strcpy(topic, MHB_USER);
//     strcat(topic, "/");
//     strcat(topic, MHB_ZONE);
//     strcat(topic, "/-/+");
//     insert_topic(topic);

//     // let s use sprintf
//     strcpy(topic, MHB_USER);
//     strcat(topic, "/-/-/+");
//     insert_topic(topic);
// }

static void  topic_received(mqtt_message_data_t *md)
{
    // char topic[128];
    char msg[1048]; // hope it s enough
    memcpy(msg, md->message->payload, md->message->payloadlen);
    msg[md->message->payloadlen] = '\0';

    md->topic->lenstring.data[md->topic->lenstring.len] = '\0';

    printf("Msg received on topic %d: %s\n", 
                         md->topic->lenstring.len, 
                         md->topic->lenstring.data); 

    // // memcpy(topic, md->message->payload, md->message->payloadlen);

    // char * action = strrchr(md->topic->lenstring.data, '/') + 1;
    // printf("action: %s\nMsg: %s\n\n", action, msg);    // , (char *)md->message->payload
    // reducer(action, msg);
}

void  mqtt_task(void *pvParameters)
{
    int ret = 0;
    struct mqtt_network network;
    uint8_t mqtt_buf[100];
    uint8_t mqtt_readbuf[100];
    mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;

    // struct MQTTMessage *pxMessage;

    mqtt_network_new( &network );

    while(1) {
        // xSemaphoreTake(wifi_alive, portMAX_DELAY);
        printf("%s: started\n\r", __func__);
        printf("%s: (Re)connecting to MQTT server %s ... ",__func__,
               MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
        if( ret ){
            printf("error: %d\n\r", ret);
            taskYIELD();
            vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 sec
            continue;
        }
        printf("done\n\r");
        mqtt_client_new(&client, &network, 5000, 
                        mqtt_buf, 200,
                        mqtt_readbuf, 200);

        data.willFlag       = 0;
        data.MQTTVersion    = 3;
        #ifdef MQTT_CLIENT_ID
        data.clientID.cstring   = MQTT_CLIENT_ID;
        #else
        data.clientID.cstring   = mqtt_client_id;
        #endif
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

        // printf("Subscribe to topic.............\n");
        mqtt_subscribe(&client, "alex/#", MQTT_QOS1, topic_received);
        // xQueueReset(publish_queue);                

        while(1){

            // while(xQueueReceive(publish_queue, &( pxMessage ), ( TickType_t ) 0) ==
            //       pdTRUE){
            //     mqtt_message_t message;
            //     message.payload = pxMessage->msg;
            //     message.payloadlen = strlen(pxMessage->msg);
            //     message.dup = 0;
            //     message.qos = MQTT_QOS1;
            //     message.retained = 1;

            //     // char * topic = pxMessage->topic;

            //     char topic[128];
            //     strcpy(topic, topics[0]);
            //     topic[strlen(topic) - 1] = '\0';
            //     strcat(topic, pxMessage->topic);                

            //     printf("got message to publish %s: %s\r\n", topic, pxMessage->msg);
            //     ret = mqtt_publish(&client, topic, &message);
            //     if (ret != MQTT_SUCCESS ){
            //         printf("error while publishing message: %d\n", ret );
            //         break;
            //     }
            //     // no need to put it here since we received on subscribe
            //     // trigger(topic, pxMessage->msg);
            // }

            ret = mqtt_yield(&client, 1000);
            if (ret == MQTT_DISCONNECTED)
                break;
        }
        printf("Connection dropped, request restart\n\r");
        mqtt_network_disconnect(&network);
        taskYIELD();
    }
}
