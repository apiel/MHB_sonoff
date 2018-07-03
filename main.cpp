#include "task.hpp"
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "ota-tftp.h"

#include <string.h>

#include <ssid_config.h>

#include "config.h"
#include "wifi.h"
#include "button.h"
#include "EEPROM.h"
#include "version.h"
#include "rf.h"
#include "relay.h"
#include "timer.h"
#include "controller.h"

#include "lwipopts.h"
#include "httpd.h"
#include "upnp.h"
#include "mqtt.h"

#ifdef PIN_DHT
    #include "thermostat.h"
    #include "dht.h"
#endif

#ifdef PIN_DS18B20
    #include "thermostat.h"
    #include "ds18b20.h"
#endif

task_rf_t task_rf;

// todo
// #include "http_client_ota.h"

// uid in eeprom

// status
// wifi task to detect disconnect, with callback
// improve wifi
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm dout -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin

extern "C" void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version: %s\n", sdk_system_get_sdk_version());
    printf("MyHomeBridge sonoff compile version: %s\n", VERSION);

    EEPROM.begin(EEPROM_SIZE);

    // wifi_new_connection((char *)WIFI_SSID, (char *)WIFI_PASS); // dev mode
    wifi_init(); // default


// Relay1.relay_toggle();
    Button button = Button(wifi_toggle, [](){ Relay1.relay_toggle(); });
    button.init();


    // on  000001000101010100111100
    // off 000001000101011100001100
    // rf_save_store((char *)"0c0000000101100101010000010b000000010110010101001000E");
    // rf_save_store((char *)"0a0100110100101100101001101b0100110100101100101001100b000000010110010101000010E"); // pir on, pir 16 sec off, right button off
    // rf_save_store((char *)"0a1010100000001100100001101b1010100000001100100001100c0000001011111110110000011b000000101111111011000001E");

// 101010000000110010000110 pir digoo
// 000000101111111011000001 single button

// higher part button on top
// 000000010110010101001000 off left button table

// 100000111011001101101000 off left button kitchen
// 100000111011001101100001 middle button kitchen
// 100000111011001101100010 toggle right button kitchen
    // rf_save_store((char *)"0c1000001110110011011000100b100000111011001101101000E");


    // 0a0100110100101100101001101b0100110100101100101001100c0000001011111110110000011b000000101111111011000001E // might be the new one
    // 000000101111111011000001
    // 0a0100110100101100101001101b0100110100101100101001100c0000000101100101010000100d1010100000001100100001101e101010000000110010000110E

    #ifdef PIN_RF433_RECEIVER
    task_rf.init_store();
    // task_rf.test();
    task_rf.task_create("task_rf");
    #endif

    #ifdef TFTP_PORT
    ota_tftp_init_server(TFTP_PORT); // not very stable because of TFTP protocol
    #endif

    xTaskCreate(&timer_task, "timer_task", 1024, NULL, 4, NULL);

    #ifdef PIN_DHT
    thermostat_init();
    xTaskCreate(dhtTask, "dhtTask", 256, NULL, 2, NULL);
    #endif

    #ifdef PIN_DS18B20
    thermostat_init();
    xTaskCreate(ds18b20Task, "ds18b20Task", 256, NULL, 2, NULL);
    #endif

    xTaskCreate(&httpd_task, "http_server", 1024, NULL, 2, NULL);
    xTaskCreate(&upnp_task, "upnp_task", 1024, NULL, 5, NULL);

    mqtt_start();
}
