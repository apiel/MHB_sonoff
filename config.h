
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEVICE_ID "MHB_"
#define DEVICE_NAME "WEMOS"

#define MHB_USER "alex"
#define MHB_ZONE "garage"

#define AP_PSK "myhomebridge" // between 8 and 63 ASCII-encoded characters

#define TFTP_PORT 69 // OTA is now over WebSocket

#define EEPROM_START 0xEB000 // it was working with 0x200000 but maybe we should increse to 0x250000
#define EEPROM_SIZE 1024 // I am not sure it is used
#define EEPROM_WIFI_MODE 0 // byte containing the mode of the WIFI store in EEPROM: STATION_MODE or SOFTAP_MODE
#define EEPROM_THERMOSTAT 1 // byte containing the temp limit of the thermostat
#define EEPROM_RF_STORE_START 10 // byte where rf storage start
#define EEPROM_RF_STORE_SIZE 512

#define WS_CLIENT_PORT 3080

#define LOG_ERROR
#define LOG_INFO
#define LOG_DEBUG

// #define SONOFF 1
#ifdef SONOFF
    #define PIN_BUTTON 0 // D3
    #define PIN_LED 13
    #define PIN_RF433_RECEIVER 14 // D5
    #define PIN_RELAY 12
#endif

// #define SONOFF4CH 1
#ifdef SONOFF4CH
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
    #define PIN_BUTTON 0 // D3
    #define PIN_LED 2
    #define PIN_RF433_RECEIVER 14 // D5
    #define PIN_RELAY 5 // D1
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
