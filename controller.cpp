#include <string.h>
#include <espressif/esp_common.h>

// #include "web.h"
#include "log.h"
#include "wifi.h"
#include "utils.h"
#include "relay.h"
#include "config.h"
#include "rf.h"
// #include "web_ota.h"
#include "timer.h"
#include "thermostat.h"
#include "action.h"
#include "mqtt.h"
#include "ota.h"

void controller_set_wifi(char * data)
{
    data += 9;

    char ssid[32], password[64];
    char * next = str_extract(data, 0, ' ', ssid);
    str_extract(next, ' ', '\0', password);
    logInfo("Set wifi ssid and password\n");

    wifi_new_connection(ssid, password);
    mqtt_send("log", "wifi configured");
}

void controller_ota_action(char * data)
{
    printf("controller_ota_action\n");
    char host[32] = OTA_HOST;
    char port[5] = OTA_PORT;
    char path[64] = OTA_PATH;

    char * next = str_extract(data, 0, ' ', host);
    if (!next) next = str_extract(data, 0, '\0', host);
    else if (next) {
        next = str_extract(data, ' ', ' ', port);
        if (!next) next = str_extract(data, ' ', '\0', port);
        else if (next) {
            str_extract(next, ' ', '\0', path);
        }
    }
    if (!strlen(host)) strcpy(host, OTA_HOST);

    // printf("host: %s, port: %s, path: %s\n", host, port, path);
    ota(host, port, path);
}

void controller_relay_send_status(Relay * relay)
{
    mqtt_send(
        relay->get_id(),
        relay->relay_status() == RELAY_ON ? "on" : "off"
    );
}

void controller_temperature_send(int16_t temperature)
{
    char value[3];
    sprintf(value, "%d%c", temperature, '\0');
    mqtt_send("temperature", (const char *)value);
}

void controller_humidity_send(int16_t humidity)
{
    char value[3];
    sprintf(value, "%d%c", humidity, '\0');
    mqtt_send("humidity", (const char *)value);
}

void controller_relay_action_timer(Action * object, int action, char * data) {
    if (data[0] == ' ') {
        data++;
        int seconds = char_to_int(data++); // maybe we should use strtol(str, (char **)NULL, 10) or atoi
        int id = 0;
        if (data[0] == ' ') {
            data++;
            id = char_to_int(data++);
        }
        if (id > 0 || seconds > 0) {
            add_timer(object, action, seconds, id);
            return;
        }
    }
    (* object)(action);
}

void controller_relay_action(Action * object, char * data)
{
    if (strncmp(data, "on", 2) == 0) {
        // relay_on();
        data += 3;
        controller_relay_action_timer(object, ACTION_RELAY_ON, data);
    } else if (strncmp(data, "off", 3) == 0) {
        // relay_off();
        data += 4;
        controller_relay_action_timer(object, ACTION_RELAY_OFF, data);
    } else if (strncmp(data, "toggle", 6) == 0) {
        // relay_toggle();
        data += 7;
        controller_relay_action_timer(object, ACTION_RELAY_TOGGLE, data);
    } else if (strncmp(data, "status", 6) == 0) {
        controller_relay_send_status((Relay *)object);
    }
}

void controller_rf_save_action(char * data)
{
    rf_save_store(data);
    mqtt_send("log", "rf saved");
}

void controller_thermostat_action(char * data)
{
    if (data[0] == ' ') {
        data++;
        int temperature = char_to_int(data++);
        set_thermostat(temperature);
    }
}

void controller_parse(char *action, char *data)
{
    if (strncmp(action, "wifi/set", 9) == 0) {
        controller_set_wifi(data);
    } else if (strncmp(action, "relay/1", 7) == 0) {
        controller_relay_action(&Relay1, data);
#ifdef PIN_RELAY_2
    } else if (strncmp(action, "relay/2", 7) == 0) {
        controller_relay_action(&Relay2, data);
#endif
#ifdef PIN_RELAY_3
    } else if (strncmp(action, "relay/3", 7) == 0) {
        controller_relay_action(&Relay3, data);
#endif
#ifdef PIN_RELAY_4
    } else if (strncmp(action, "relay/4", 7) == 0) {
        controller_relay_action(&Relay4, data);
#endif
    } else if (strncmp(action, "relay", 5) == 0) {
        printf("relay default\n");
        controller_relay_action(&Relay1, data);
    } else if (strncmp(action, "rf/save", 7) == 0) {
        controller_rf_save_action(data);
    } else if (strncmp(action, "ota", 3) == 0) {
        controller_ota_action(data);
    } else if (strncmp(action, "thermostat", 10) == 0) {
        controller_thermostat_action(data);
    }
}
