
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define DEVICE_ID "MHB_"
#define VERSION "0.0.0" 

#define MHB_USER "alex"
#define MHB_ZONE "garage"

#define AP_PSK "myhomebridge" // between 8 and 63 ASCII-encoded characters

#define EEPROM_START 0x200000
#define EEPROM_SIZE 512
#define EEPROM_WIFI_MODE 0 // byte containing the mode of the WIFI store in EEPROM: STATION_MODE or SOFTAP_MODE

#define HTTPD_PORT 80 // cannot use the same port as wget
#define WS_CLIENT_PORT 8080

#define LOG_ERROR
#define LOG_INFO
// #define LOG_DEBUG

#define MQTT_USER NULL
#define MQTT_PASS NULL
// #define MQTT_HOST ("vps.alexparadise.com")
#define MQTT_HOST ("192.168.1.106")
// #define MQTT_PORT 1883

#define PIN_BUTTON 14 // D5
#define PIN_LED 2

// https://community.blynk.cc/uploads/default/original/2X/4/4f9e2245bf4f6698e10530b9060595c893bf49a2.png
// D0 > GPIO 16
// D1 > GPIO 5
// D2 > GPIO 4 
// D3 > GPIO 0 // when emitter connected bug on boot
// D4 > GPIO 2 // when emitter connected bug on boot
// D5 > GPIO 14
// D6 > GPIO 12
// D7 > GPIO 13
// D8 > GPIO 15

// #define PIN_RF433_EMITTER 4
// #define PIN_RF433_RECEIVER 12
// #define PIN_DHT 5
// #define PIN_PIR 4
// #define PHOTORESISTOR

// #define DHT_TYPE DHT_TYPE_DHT22
// #define DHT_TYPE DHT_TYPE_DHT11

// #define UPNP

#endif
