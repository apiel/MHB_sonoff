#include <string.h>

#include <esplibs/libnet80211.h> // wifi off
#include <espressif/esp_common.h>
#include <dhcpserver.h>
#include <esp8266.h>

#include "config.h"
#include "wifi.h"
#include "led.h"

// We should deactivate sofap after connection to wifi
// Use MAC address to generate SoftAP SSID

void wifi_off(void)
{
    sdk_wifi_station_disconnect();
    // sdk_wifi_station_stop();
    wifi_access_point_off();
    // sdk_wifi_softap_stop();
}

void wifi_init(void)
{
  wifi_connect();
//   wifi_access_point();
}

void wifi_new_connection(char * ssid, char * password)
{
    printf("Connect to new wifi: %s %s", ssid, password);
    
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

    sdk_wifi_station_set_config(&config);
    // sdk_wifi_station_disconnect();
    sdk_wifi_station_connect();
}

void wifi_connect(void)
{
    sdk_wifi_set_opmode(STATION_MODE); 

    struct sdk_station_config config;
    bool ret = sdk_wifi_station_get_config(&config);
    if(ret) printf("existing wifi settings: ssid = %s, password = %s\n", config.ssid, config.password);
    else printf("no wifi settings founds: ssid = %s, password = %s\n", config.ssid, config.password);

    // sdk_wifi_station_disconnect();
    sdk_wifi_station_connect();  
}

void wifi_toggle(void)
{
    printf("Wifi toggle\n");
    led_blink(10, 100);
    wifi_off();
    if (sdk_wifi_get_opmode() == SOFTAP_MODE) {
        wifi_connect();
    } else {
        wifi_access_point();
    }
}

void wifi_access_point(void)
{
    printf("Activate access point\n");
    sdk_wifi_set_opmode(SOFTAP_MODE);
    struct ip_info ap_ip;
    IP4_ADDR(&ap_ip.ip, 172, 16, 0, 1);
    IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
    IP4_ADDR(&ap_ip.netmask, 255, 255, 0, 0);
    sdk_wifi_set_ip_info(1, &ap_ip);

    struct sdk_softap_config ap_config;
    ap_config.ssid_hidden = 0;
    ap_config.authmode = AUTH_WPA_WPA2_PSK;
    ap_config.max_connection = 1;
    strcpy((char*)(ap_config.password), AP_PSK);
    strcpy((char*)(ap_config.ssid), get_uid());
    sdk_wifi_softap_set_config(&ap_config);

    ip_addr_t first_client_ip;
    IP4_ADDR(&first_client_ip, 172, 16, 0, 2);
    dhcpserver_start(&first_client_ip, 4);     
}

void wifi_access_point_off(void)
{
    struct sdk_softap_config ap_config;
    *ap_config.ssid = 0;
    *ap_config.password = 0;

    sdk_wifi_softap_set_config(&ap_config);  
    dhcpserver_stop();  
}

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
        printf("Device unique id: %s\n", uid);
    }

    return uid;
}