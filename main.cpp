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

// make own rf receiver (rcswitch not recurrent enough)
// improve httpd, maybe use lib :p

// maybe get rid of mqtt?
// uid in eeprom

// status
// wifi task to detect disconnect, with callback
// improve wifi
// sonoff 4 channel
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm dout -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin
// new sonoff basic v1.1
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


    // >> table switch
    // ZZVUVIJZ-0 left
    // ZZVUVIZJ-0 midlle
    // ZZVUVIZV-0 right

    // rf_save_store((char *)"b00ZZVUVIJZ-0#c00ZZVUVIZV-0#"); // table light

    #ifdef PIN_RF433_RECEIVER
    rf.init();
    // rf.test();
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
