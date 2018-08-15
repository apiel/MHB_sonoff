#include <string.h>

#include <esplibs/libnet80211.h> // wifi off
#include <espressif/esp_common.h>

#include <dhcpserver.h>
#include <esp8266.h>

#include "config.h"
#include "wifi.h"
#include "wifiSTA.h"
#include "led.h"

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
    static char uid[20];
    static bool uid_done = false;
    if (!uid_done) {
        memset(uid, 0, sizeof(uid));
        strcpy(uid, DEVICE_ID);
#ifdef DEVICE_NAME
        strcat(uid, DEVICE_NAME);
#else
        strcat(uid, get_mac_uid());
#endif
        uid_done = true;
        printf("-> Device unique id: %s\n", uid);
    }

    return uid;
}