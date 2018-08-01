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
        printf(".\n"); // we could show the status
        taskYIELD();
        vTaskDelay(100);
    }
    if (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
        sdk_system_restart();
    }
    printf("\nConnected to wifi\n");
}

void wifi_init(void)
{
    int mode = EEPROM.read(EEPROM_WIFI_MODE);
    // logDebug("eeprom val %d ap %d station %d\n", mode, SOFTAP_MODE, STATION_MODE);

    if (mode == SOFTAP_MODE) {
        wifi_access_point();
    } else {
        wifi_sta_connect();
    }
}

void wifi_toggle(void)
{
    logInfo("Wifi toggle\n");
    if (sdk_wifi_get_opmode() == SOFTAP_MODE) {
        EEPROM.write(EEPROM_WIFI_MODE, STATION_MODE);
    } else {
        EEPROM.write(EEPROM_WIFI_MODE, SOFTAP_MODE);
    }
    EEPROM.commit();

    led_blink(10, 100);
    sdk_system_restart();
}

void wifi_new_connection(char * ssid, char * password)
{
    EEPROM.write(EEPROM_WIFI_MODE, STATION_MODE); // we might have to find a central place for this with wifi toggle
    EEPROM.commit();
    wifi_sta_new_connection(ssid, password);
}

// does it make sense to make it access point...
// we could always create a shared wifi network on the laptop or on mobile...
// if we want to keep it, we could use the retry from wait connection to switch to access point
void wifi_access_point(void)
{
    logInfo("Activate access point\n");
    sdk_wifi_set_opmode(SOFTAP_MODE);
    struct ip_info ap_ip;
    IP4_ADDR(&ap_ip.ip, 192, 168, 0, 1);
    IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    IP4_ADDR(&ap_ip.netmask, 255, 255, 255, 0);
    sdk_wifi_set_ip_info(1, &ap_ip);

    struct sdk_softap_config ap_config;
    ap_config.ssid_hidden = 0;
    ap_config.authmode = AUTH_WPA_WPA2_PSK;
    ap_config.max_connection = 1;
    strcpy((char*)(ap_config.password), AP_PSK);
    strcpy((char*)(ap_config.ssid), get_uid());
    sdk_wifi_softap_set_config(&ap_config);

    ip_addr_t first_client_ip;
    IP4_ADDR(&first_client_ip, 192, 168, 0, 2);
    dhcpserver_start(&first_client_ip, 4);
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
            if (bit == '#') break;
            uid[pos] = bit;
        }
        printf("UID from EEPROM %s (%d)\n", uid, strlen(uid));
#endif

        if (strlen(uid) == 0) {
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