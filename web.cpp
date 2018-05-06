#include <lwip/api.h> // vtask
#include <lwip/debug.h>
#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>
#include <semphr.h>

#include "web.h"
#include "log.h"
#include "wifi.h"
#include "utils.h"
#include "relay.h"
#include "config.h"
#include "rf.h"
#include "web_client.h"
#include "web_ota.h"
#include "timer.h"
#include "thermostat.h"
#include "action.h"

int web_close(struct tcp_pcb *pcb)
{
    int err = ERR_OK;
    if (pcb) {
        logDebug("Web close\n"); 
        tcp_recv(pcb, NULL);
        tcp_poll(pcb, NULL, 0);
        err = tcp_close(pcb);
    }
    return err;
}

void web_ws_read(struct wsMessage * msg)
{
    msg->opcode = (*msg->data) & 0x0F;
    msg->data += 1;
    // msg->isMasked = (*msg->data) & (1<<7);
    msg->isMasked = ((*msg->data) & 0x80) == 0x80;
    msg->len = (*msg->data) & 0x7F;
    msg->data += 1;

    if (msg->len == 126) { // messages > 125 but <= 65536 bytes
        msg->len = msg->data[0] << 8 | msg->data[1];
        msg->data += 2;
    } else if (msg->len == 127) {
        logDebug("ws msg 65536 bits not supported\n");
    }
    if (msg->opcode == OPCODE_TEXT) {
        msg->data[msg->len] = '\0';
    }

    // printf("opcode %d len %d ismasked %d\n", msg->opcode, msg->len, msg->isMasked);
}

char * web_ws_encode_msg(char * data, unsigned int opcode)
{
  static char buf[512];
  int len = strlen(data);

  int offset = 2;
  buf[0] = 0x80 + opcode;
  if (len > 125) { // my not be necessary (need to see RF)
    offset = 4;
    buf[1] = 126;
    buf[2] = len >> 8;
    buf[3] = len;
  } else {
    buf[1] = len;
  }

  memcpy(&buf[offset], data, len);
  buf[len+offset] = '\0';

  return buf;
}

SemaphoreHandle_t mutex = xSemaphoreCreateMutex(); // I am not sure this make a difference
void web_ws_send(struct tcp_pcb *pcb, char *msg, unsigned int opcode)
{
    if (pcb && xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        msg = web_ws_encode_msg(msg, opcode);
        err_t err = tcp_write(pcb, msg, strlen(msg), 0);
        // printf("tcpwrite done %d %s\n", err, msg);
        LWIP_ASSERT("Something went wrong while tcp write to client", err == ERR_OK);
        xSemaphoreGive(mutex);
    }
}

void web_ws_set_wifi(char * data)
{
    data += 9;

    char ssid[32], password[64];
    char * next = str_extract(data, 0, ' ', ssid);
    str_extract(next, ' ', '\0', password);
    logInfo("Set wifi ssid and password\n");

    wifi_new_connection(ssid, password);
    web_send_all((char *)". wifi configured");
}

void web_ws_relay_send_status()
{
    if (Relay1.relay_status() == RELAY_ON) {
        web_send_all((char *)". relay on");
    } else {
        web_send_all((char *)". relay off");
    }
}

void web_ws_temperature_send(int16_t temperature)
{
    char msg[17];
    sprintf(msg, ". temperature %d%c", temperature, '\0');
    web_send_all(msg);
    // web_send_all((char *)". temperature 10");
}

void web_ws_humidity_send(int16_t humidity)
{
    char msg[14];
    sprintf(msg, ". humidity %d%c", humidity, '\0');
    web_send_all(msg);
    // web_send_all((char *)". humidity 10");
}

void web_ws_relay_action_timer(Action * object, int action, char * data) {
    if (data[0] == ' ') {
        data++;
        int seconds = char_to_int(data++);
        if (seconds > 0) {
            // we also might need a way to have an id to cancel timer?
            // we might then use (int) strtol(str, (char **)NULL, 10) or atoi
            // or we could use pointer as well
            add_timer(object, action, seconds);
            return;
        }
    }
    (* object)(action);
}

void web_ws_relay_action(Action * object, char * data)
{
    data += 5;
    if (strncmp(data, " on", 3) == 0) {
        // relay_on();
        data += 3;
        web_ws_relay_action_timer(object, ACTION_RELAY_ON, data);
    } else if (strncmp(data, " off", 4) == 0) {
        // relay_off();
        data += 4;
        web_ws_relay_action_timer(object, ACTION_RELAY_OFF, data);
    } else if (strncmp(data, " toggle", 7) == 0) {
        // relay_toggle();
        data += 7;
        web_ws_relay_action_timer(object, ACTION_RELAY_TOGGLE, data);
    } else if (strncmp(data, " status", 6) == 0) {
        web_ws_relay_send_status();
    }
}

void web_ws_rf_save_action(char * data)
{
    data += 7;
    rf_save_store(data);
    web_send_all((char *)". rf saved");
}

void web_ws_ota_action(char * data)
{
    data += 3;
    if (strncmp(data, " start", 6) == 0) {
        web_ota_start();
    } else if (strncmp(data, " end", 4) == 0) {
        web_ota_end();
    } else if (strncmp(data, " next", 5) == 0) {
        web_ota_next();
    } else if (strncmp(data, " slot", 5) == 0) {
        data += 5;
        if (data[0] == '\0') {
            web_ota_get_slot();
        } else {
            web_ota_set_slot(data[0]);
        }
    }
}

void web_ws_thermostat_action(char * data)
{
    data += 10;
    if (data[0] == ' ') {
        data++;
        int temperature = char_to_int(data++);
        set_thermostat(temperature);
    }
}

void web_ws_parse(char *data)
{
    if (strncmp(data, "wifi/set ", 9) == 0) {
        web_ws_set_wifi(data);
    } else if (strncmp(data, "relay/1", 7) == 0) {
        web_ws_relay_action(&Relay1, data);
#ifdef PIN_RELAY_2
    } else if (strncmp(data, "relay/2", 7) == 0) {
        web_ws_relay_action(&Relay2, data);
#endif
#ifdef PIN_RELAY_3
    } else if (strncmp(data, "relay/3", 7) == 0) {
        web_ws_relay_action(&Relay3, data);
#endif
#ifdef PIN_RELAY_4
    } else if (strncmp(data, "relay/4", 7) == 0) {
        web_ws_relay_action(&Relay4, data);
#endif
    } else if (strncmp(data, "relay", 5) == 0) {
        web_ws_relay_action(&Relay1, data);
    } else if (strncmp(data, "rf/save", 7) == 0) {
        web_ws_rf_save_action(data);
    } else if (strncmp(data, "ota", 3) == 0) {
        web_ws_ota_action(data);
    } else if (strncmp(data, "thermostat", 10) == 0) {
        web_ws_thermostat_action(data);
    }
}

void web_send_all(char * msg)
{
    if (web_client_ws_is_connected()) {
        // logInfo("Send msg: %s\n", msg);
        printf("# Send msg: %s\n", msg); // logInfo does not work
        web_client_ws_send(msg);
    }
}
