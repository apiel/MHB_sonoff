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
    data += 5;
    if (strncmp(data, " on", 3) == 0) {
        // relay_on();
        data += 3;
        controller_relay_action_timer(object, ACTION_RELAY_ON, data);
    } else if (strncmp(data, " off", 4) == 0) {
        // relay_off();
        data += 4;
        controller_relay_action_timer(object, ACTION_RELAY_OFF, data);
    } else if (strncmp(data, " toggle", 7) == 0) {
        // relay_toggle();
        data += 7;
        controller_relay_action_timer(object, ACTION_RELAY_TOGGLE, data);
    } else if (strncmp(data, " status", 6) == 0) {
        controller_relay_send_status((Relay *)object);
    }
}

void controller_rf_save_action(char * data)
{
    data += 7;
    rf_save_store(data);
    mqtt_send("log", "rf saved");
}

// void controller_ota_action(char * data)
// {
//     data += 3;
//     if (strncmp(data, " start", 6) == 0) {
//         web_ota_start();
//     } else if (strncmp(data, " end", 4) == 0) {
//         web_ota_end();
//     } else if (strncmp(data, " next", 5) == 0) {
//         web_ota_next();
//     } else if (strncmp(data, " slot", 5) == 0) {
//         data += 5;
//         if (data[0] == '\0') {
//             web_ota_get_slot();
//         } else {
//             web_ota_set_slot(data[0]);
//         }
//     }
// }

void controller_thermostat_action(char * data)
{
    data += 10;
    if (data[0] == ' ') {
        data++;
        int temperature = char_to_int(data++);
        set_thermostat(temperature);
    }
}

void controller_parse(char *data)
{
    // printf("data: %s\n", data);
    if (strncmp(data, "wifi/set ", 9) == 0) {
        controller_set_wifi(data);
    } else if (strncmp(data, "relay/1", 7) == 0) {
        data += 2;
        controller_relay_action(&Relay1, data);
#ifdef PIN_RELAY_2
    } else if (strncmp(data, "relay/2", 7) == 0) {
        data += 2;
        controller_relay_action(&Relay2, data);
#endif
#ifdef PIN_RELAY_3
    } else if (strncmp(data, "relay/3", 7) == 0) {
        data += 2;
        controller_relay_action(&Relay3, data);
#endif
#ifdef PIN_RELAY_4
    } else if (strncmp(data, "relay/4", 7) == 0) {
        data += 2;
        controller_relay_action(&Relay4, data);
#endif
    } else if (strncmp(data, "relay", 5) == 0) {
        printf("relay default\n");
        controller_relay_action(&Relay1, data);
    } else if (strncmp(data, "rf/save", 7) == 0) {
        controller_rf_save_action(data);
    // } else if (strncmp(data, "ota", 3) == 0) {
    //     controller_ota_action(data);
    } else if (strncmp(data, "thermostat", 10) == 0) {
        controller_thermostat_action(data);
    }
}
