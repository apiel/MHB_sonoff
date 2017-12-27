#include <string.h>

#include <esplibs/libnet80211.h> // wifi off
#include <espressif/esp_common.h>
#include <dhcpserver.h>
#include <esp8266.h>

#include "config.h"
#include "wifi.h"
#include "led.h"
#include "log.h"
#include "EEPROM.h"

void wifi_init(void)
{
    int mode = EEPROM.read(EEPROM_WIFI_MODE);
    logInfo("eeprom val %d ap %d station %d\n", mode, SOFTAP_MODE, STATION_MODE);
    
    if (mode == SOFTAP_MODE) {
        wifi_access_point();        
    } else {
        wifi_connect();
    }  
}

void wifi_toggle(void)
{
    logInfo("Wifi toggle\n");
    led_blink(10, 100);
    if (sdk_wifi_get_opmode() == SOFTAP_MODE) {
        EEPROM.write(EEPROM_WIFI_MODE, STATION_MODE);
    } else {
        EEPROM.write(EEPROM_WIFI_MODE, SOFTAP_MODE);
    }
    EEPROM.commit();
    sdk_system_restart();
}

void wifi_new_connection(char * ssid, char * password)
{
    printf("Connect to new wifi ssid: %s pwd: %s\n", ssid, password);
    
    struct sdk_station_config config;    
    if(ssid != NULL) {
        strcpy((char*)(config.ssid), ssid);
        
        if(password != NULL) {
            strcpy((char*)(config.password), password);
        }
        else {
            config.password[0] = '\0';
        }
    }
    else {
        config.ssid[0]     = '\0';
        config.password[0] = '\0';
    }    

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    EEPROM.write(EEPROM_WIFI_MODE, STATION_MODE); // we might have to find a central place for this with wifi toggle
    sdk_wifi_station_connect();
}

void wifi_connect(void)
{
    sdk_wifi_set_opmode(STATION_MODE); 

    struct sdk_station_config config;
    bool ret = sdk_wifi_station_get_config(&config);
    if(ret) logInfo("existing wifi settings: ssid = %s, password = %s\n", config.ssid, config.password);
    else logInfo("no wifi settings founds: ssid = %s, password = %s\n", config.ssid, config.password);

    // sdk_wifi_station_disconnect();
    sdk_wifi_station_connect();  
}

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

// this could go in utils
const char * get_uid(void)
{
    // Use MAC address for Station as unique ID
    static char uid[20];
    char mac[13];
    static bool uid_done = false;
    int8_t i;
    uint8_t x;
    if (!uid_done) {
        memset(uid, 0, sizeof(uid));
        strcpy(uid, DEVICE_ID);

        if (!sdk_wifi_get_macaddr(STATION_IF, (uint8_t *)mac)) {
            strcat(uid, "generic"); 
        }
        else {
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
            strcat(uid, mac); 
        }
        uid_done = true;
        logInfo("Device unique id: %s\n", uid);
    }

    return uid;
}