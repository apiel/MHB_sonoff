#include <string.h>

#include <esplibs/libnet80211.h> // wifi off
#include <espressif/esp_common.h>

#include <dhcpserver.h>
#include <esp8266.h>

#include "config.h"
#include "wifi.h"
#include "wifiSTA.h"
#include "led.h"
#include "log.h"
#include "EEPROM.h"

void wifi_wait_connection(void)
{
    for(int retry = 20; retry > 0; retry--) {
        if (sdk_wifi_station_get_connect_status() == STATION_GOT_IP) {
            break;
        }
        task_led_blink(2, 5);
        printf(".\n"); // we could show the status
        taskYIELD();
        vTaskDelay(100);
    }
    if (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
        sdk_system_restart();
    }
    // printf("\nConnected to wifi\n");
}

void wifi_init(void)
{
    wifi_sta_connect();
}

void wifi_new_connection(char * ssid, char * password)
{
    wifi_sta_new_connection(ssid, password);
}

void save_uid(char * data)
{
#ifdef EEPROM_UID_START
    EEPROM.save(EEPROM_UID_START, (uint8_t *)data, strlen(data));
#endif
}

const char * get_mac_uid()
{
    static char mac[13];

    int8_t i;
    uint8_t x;
    if (!sdk_wifi_get_macaddr(STATION_IF, (uint8_t *) mac))
        return NULL;
    for (i = 5; i >= 0; --i)
    {
        x = mac[i] & 0x0F;
        if (x > 9) x += 7;
        mac[i * 2 + 1] = x + '0';
        x = mac[i] >> 4;
        if (x > 9) x += 7;
        mac[i * 2] = x + '0';
    }
    mac[12] = '\0';
    return mac;
}

// this could go in utils
const char * get_uid(void)
{
    // Use MAC address for Station as unique ID
    static char uid[EEPROM_UID_SIZE];
    static bool uid_done = false;
    if (!uid_done) {
#ifdef EEPROM_UID_START
        for(int pos = 0; pos < EEPROM_UID_SIZE; pos++) {
            int bit = EEPROM.read(EEPROM_UID_START + pos);
            if (bit == '#') { uid_done = true; break; }
            uid[pos] = bit;
        }
        printf("UID from EEPROM %s (%d)\n", uid, strlen(uid));
#endif

        if (!uid_done || strlen(uid) == 0) {
            uid_done = false;
            memset(uid, 0, sizeof(uid));
            strcpy(uid, DEVICE_ID);
#ifdef DEVICE_NAME
            strcat(uid, DEVICE_NAME);
#else
            strcat(uid, get_mac_uid());
#endif
        }
        uid_done = true;
        printf("-> Device unique id: %s\n", uid);
    }

    return uid;
}