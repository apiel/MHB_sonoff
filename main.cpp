#include "task.hpp"
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "ota-tftp.h"

#include <string.h>

#include <ssid_config.h>

#include "config.h"
// #include "wifi.h"
#include "wifiSTA.h"
#include "button.h"
#include "web_server.h"
#include "web_client.h"
#include "EEPROM.h"
#include "version.h"
#include "rf.h"
#include "relay.h"

task_rf_t task_rf;

// todo, remove httpd server, only ws client
// improve wifi

extern "C" void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version: %s\n", sdk_system_get_sdk_version());
    printf("MyHomeBridge sonoff compile version: %s\n", VERSION);

    EEPROM.begin(EEPROM_SIZE);

    #ifdef PIN_RELAY
    relay_init();
    #endif

    // WiFi.mode(WIFI_STA);
    // WiFi.begin("alex2", "SuperStar86");
    
    // // ets_isr_mask((1<<5));
    // sdk_wifi_station_connect();
    // // ets_isr_unmask((1<<5));

    wifi_sta_new_connection((char *)WIFI_SSID, (char *)WIFI_PASS);
    // wifi_new_connection((char *)WIFI_SSID, (char *)WIFI_PASS); // dev mode
    // wifi_new_connection((char *)"alex", (char *)WIFI_PASS); // dev mode
    // wifi_init(); // default

    // Button button = Button(wifi_toggle);
    // button.init();


    // on  000001000101010100111100
    // off 000001000101011100001100
    // rf_save_store("a000001000101010100111100b000001000101011100001100E");

/*
    task_rf.init_store();
    task_rf.test();
    task_rf.task_create("task_rf");

    #ifdef TFTP_PORT
    ota_tftp_init_server(TFTP_PORT);
    #endif

    // need to convert to cpp

    // we should create a onwificonnectevent
    #ifdef WS_CLIENT_PORT
    xTaskCreate(&web_client_task, "web_client_task", 1024, NULL, 3, NULL);
    #endif 

    #ifdef HTTPD_PORT
    xTaskCreate(&web_server_task, "web_server_task", 1024, NULL, 2, NULL);
    #endif
*/    
}
