
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define USE_ABOOT
#define DEVICE_ID "MHB_"
#define DEVICE_NAME "WEMOS"

#define RELAY_NAME "Wemos light"
#define RELAY_2_NAME "Wemos2 light"

#define MHB_USER "alex" // this is not used
#define MHB_ZONE "garage" // not used

#define AP_PSK "myhomebridge" // between 8 and 63 ASCII-encoded characters

#define MQTT_HOST ("vps.alexparadise.com")
#define MQTT_PORT 1883
#define MQTT_USER NULL
#define MQTT_PASS NULL

#define TFTP_PORT 69

#define EEPROM_START 0xBC000 // 0x55000 + 0x55000 + 0x2000 + 0x10000 = 0xBC000
// 0x55000: rom size and we have 2 rom, 0x2000: first rom address, 0x10000: let's keep some extra space in case
// we could also use if we use aboot do:
//               (rboot_config.roms[1]-rboot_config.roms[0])*rboot_config.count + rboot_config.roms[0] (+0x10000)
// it's the same (    0x57000         -     0x2000         )*    2              + 0x2000   (+0x10000) = 0xBC000
#define EEPROM_SIZE 1024 // I am not sure it is used
#define EEPROM_WIFI_MODE 0 // byte containing the mode of the WIFI store in EEPROM: STATION_MODE or SOFTAP_MODE
#define EEPROM_THERMOSTAT 1 // byte containing the temp limit of the thermostat
#define EEPROM_RF_STORE_START 10 // byte where rf storage start
#define EEPROM_RF_STORE_SIZE 512
#define EEPROM_UID_START EEPROM_RF_STORE_SIZE + EEPROM_RF_STORE_START + 1 // byte where uid storage start
#define EEPROM_UID_SIZE 20

#define WS_CLIENT_PORT 3080

#define OTA_HOST "vps.alexparadise.com"
#define OTA_PORT "8080"
#define OTA_PATH "/firmware.bin"

#define LOG_ERROR
#define LOG_INFO
#define LOG_DEBUG

// #define SONOFF 1
#ifdef SONOFF
    // #define EEPROM_START 0xFD8F0 // 8mb 0x100000-10000 so there 10000 bytes space
    #define PIN_BUTTON 0 // D3
    #define PIN_LED 13
    #define PIN_RF433_RECEIVER 14 // D5
    #define PIN_RELAY 12
#endif

// #define SONOFF4CH 1
#ifdef SONOFF4CH
    // #define EEPROM_START 0xFD8F0 // 8mb? 0x100000-10000 so there 10000 bytes space
    #define PIN_BUTTON 0
    #define PIN_LED 13
    #define PIN_RF433_RECEIVER 2
    #define PIN_RELAY 12
    #define PIN_RELAY_2 5
    #define PIN_RELAY_3 4
    #define PIN_RELAY_4 15
#endif

#define WEMOS 1
#ifdef WEMOS
    // #define EEPROM_START 0x1FD8F0 // 16mb 0x200000-10000 so there 10000 bytes space
    #define PIN_BUTTON 0 // D3
    #define PIN_LED 2
    #define PIN_RF433_RECEIVER 14 // D5
    #define PIN_RELAY 5 // D1
    // #define PIN_RELAY_2 5
    // #define PIN_DHT 12 // D6
    // #define PIN_DS18B20 12 // D6
#endif

#define RELAY_ON 1
#define RELAY_OFF 0

// https://community.blynk.cc/uploads/default/original/2X/4/4f9e2245bf4f6698e10530b9060595c893bf49a2.png
// D0 > GPIO 16
// D1 > GPIO 5
// D2 > GPIO 4 // WEMOS MINI LED
// D3 > GPIO 0 // when emitter connected bug on boot
// D4 > GPIO 2 // when emitter connected bug on boot
// D5 > GPIO 14
// D6 > GPIO 12
// D7 > GPIO 13
// D8 > GPIO 15

// #define UPNP

#endif
