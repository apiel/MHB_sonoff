#include "task.hpp"
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include <ssid_config.h>

#include "config.h"
#include "wifi.h"
#include "button.h"
#include "version.h"
#include "rf.h"
#include "relay.h"
#include "led.h"
#include "timer.h"

#include "lwipopts.h"
#include "httpd.h"
#include "upnp.h"

// sonoff 4 channel
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm dout -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin
// new sonoff basic v1.1
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm dout -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin

// wemos
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm qio -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware_prebuilt/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin 
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm qio -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware_prebuilt/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x82000 ./firmware/firmware.bin 

// sonoff last sector at 1047280 so its: make flash FLASH_SIZE=8 FLASH_MODE=dout
// sonoff
// make flash FLASH_SIZE=8 FLASH_MODE=dout RBOOT_BIN=../aboot/firmware/aboot.bin && cu -l /dev/ttyUSB0 -s 115200

static void  main_task(void *pvParameters)
{
    wifi_wait_connection();

    // xTaskCreate(&timer_task, "timer_task", 1024, NULL, 4, NULL);
    // xTaskCreate(&httpd_task, "http_server", 1024, NULL, 2, NULL);
    // xTaskCreate(&upnp_task, "upnp_task", 1024, NULL, 5, NULL);

    while(1) { // keep task running else program crash, we could also use xSemaphore
        task_led_blink(2, 10, 20);
        taskYIELD();
        vTaskDelay(200);
        taskYIELD();
        wifi_wait_connection(); // here we could check for wifi connection and reboot if disconnect
    }
}

extern "C" void user_init(void)
{
    uart_set_baud(0, 115200);

    printf("SDK version: %s\n", sdk_system_get_sdk_version());
    printf("MyHomeBridge sonoff compile version: %s\n", VERSION);

    wifi_new_connection((char *)WIFI_SSID, (char *)WIFI_PASS); // dev mode
    // wifi_init(); // default

    Button button = Button(sdk_system_restart, [](){ Relay1.relay_toggle(); });
    button.init();

    xTaskCreate(&main_task, "main_task", 1024, NULL, 9, NULL);
    xTaskCreate(&rf_task, "rf_task", 1024, NULL, 4, NULL);
}
