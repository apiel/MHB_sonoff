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

#include "ota.h"



// find out why web ota is not working
// implement a rf protocol finder
// improve httpd, maybe use lib :p

// maybe get rid of mqtt?
// uid in eeprom

// use wait_connection retry for Access point feature
// long press would then be reboot

// status
// wifi task to detect disconnect, with callback
// improve wifi
// sonoff 4 channel
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm dout -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin
// new sonoff basic v1.1
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm dout -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin

// wemos
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm qio -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware_prebuilt/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x2000 ./firmware/firmware.bin 
// esptool.py -p /dev/ttyUSB0 --baud 115200 write_flash -fs 16m -fm qio -ff 40m 0x0 ../esp-open-rtos/bootloader/firmware_prebuilt/rboot.bin 0x1000 ../esp-open-rtos/bootloader/firmware_prebuilt/blank_config.bin 0x82000 ./firmware/firmware.bin 


// #include "espnow.h"

// sonoff last sector at 1047280 so its: make flash FLASH_SIZE=8 FLASH_MODE=dout

static void  main_task(void *pvParameters)
{
    wifi_wait_connection();

    // esp_now_set_self_role();

// let s try to download OTA here... after connection
    printf("> try ota\n");
    ota((char *)"192.168.0.179", OTA_PORT, (char *)"/firmware285.bin");
    printf("ota finished.\n");


    // >> table switch
    // ZZVUVIJZ-0 left
    // ZZVUVIZJ-0 midlle
    // ZZVUVIZV-0 right

    // rf_save_store((char *)"b00ZZVUVIJZ-0#c00ZZVUVIZV-0#"); // table light
    // rf_save_store((char *)"b00ZZVUVIJZ-0#c00ZZVUVIZJ-0#"); // sofa light

    // >> room switch
    // YZEFJIIZ-0 left
    // YZEFJIYJ-0 middle
    // YZEFJIYV-0 right

    // rf_save_store((char *)"c00YZEFJIYV-0#"); // room small light

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

    while(1) { // keep task running else program crash, we could also use xSemaphore
        vTaskDelay(1000);
        taskYIELD();
    }
}

extern "C" void user_init(void)
{
    uart_set_baud(0, 115200);

    printf("SDK version: %s\n", sdk_system_get_sdk_version());
    printf("MyHomeBridge sonoff compile version: %s\n", VERSION);

    ota_prepare();

    EEPROM.begin(EEPROM_SIZE);

    wifi_new_connection((char *)WIFI_SSID, (char *)WIFI_PASS); // dev mode
    // wifi_init(); // default

// // Relay1.relay_toggle();
//     Button button = Button(wifi_toggle, [](){ Relay1.relay_toggle(); });
//     button.init();

    xTaskCreate(&main_task, "main_task", 1024, NULL, 1, NULL);
}






    // #define xt_rsil(level) (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," __STRINGIFY(level) : "=a" (state)); state;}))
    // #define interrupts() xt_rsil(0)
    // #define noInterrupts() xt_rsil(15)

    // // erase sector to be able to write
    // uint32_t sector = rboot_config.roms[1] / SECTOR_SIZE; // rboot_config.roms[rboot_config.current_rom+1%rboot_config.count]
    // uint32_t sector_end = sector + 80; // 327680 bytes
    // for(; sector < sector_end; sector++) {
    //     printf("sdk_spi_flash_erase_sector %d\n", sector);
    //     noInterrupts();
    //     if (sdk_spi_flash_erase_sector(sector) == SPI_FLASH_RESULT_OK) {
    //         printf("Could not erase sector %d!\n", sector);
    //     }
    //     interrupts();
    //     printf("sdk_spi_flash_erase_sector done %d\n", sector);
    // } // doesnt seem to work


    // for(int test = 310000; test < 0x2000000; test += SPI_FLASH_SEC_SIZE) {
    //     // noInterrupts();
    //     if(sdk_spi_flash_erase_sector(test/SPI_FLASH_SEC_SIZE) == SPI_FLASH_RESULT_OK) {
    //         printf("%d(%d),", test, test/SPI_FLASH_SEC_SIZE);
    //     } else {
    //         printf("\n\nstop at %d\n\n", test);
    //         break;
    //     }
    //     // interrupts();
    // }
    // uint32_t yo[2];
    // yo[0] = 'A';
    // yo[1] = 'P';
    // for(int test = 310000; test < 0x2000000; test += SPI_FLASH_SEC_SIZE) {
    //     if(sdk_spi_flash_write(test, yo, sizeof(yo)) == SPI_FLASH_RESULT_OK) {
    //         printf("%d,", test);
    //     } else {
    //         printf("\n\nstop at %d\n\n", test);
    //         break;
    //     }
    // }

    // const char * data = "Alex super star";

    //     if(EEPROM.save(1, (uint8_t *)data, strlen(data))) {
    //         printf("%d,", 1);
    //     } else {
    //         printf("\n\nstop at %d\n\n", 1);
    //     }

    // for(int test = 1; test < 0x2000000; test += strlen(data)) {
    //     if(EEPROM.save(test, (uint8_t *)data, strlen(data))) {
    //         printf("%d,", test);
    //     } else {
    //         printf("\n\nstop at %d\n\n", test);
    //         break;
    //     }
    // }