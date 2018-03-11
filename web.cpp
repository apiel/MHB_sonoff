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
    logInfo("Set wifi ssid: %s password: %s\n\n", ssid, password);

    wifi_new_connection(ssid, password);
    web_send_all((char *)". wifi configured");
}

void web_ws_relay_send_status()
{
    if (relay_status() == RELAY_ON) {
        web_send_all((char *)". relay on");
    } else {
        web_send_all((char *)". relay off");
    }
}

void web_ws_relay_action_timer(void (*callback)(void), char * data) {
    if (data[0] == ' ') {
        data++;
        int seconds = char_to_int(data++);
        if (seconds > 0) {
            // we also might need a way to have an id to cancel timer?
            // we might then use (int) strtol(str, (char **)NULL, 10) or atoi
            // or we could use pointer as well
            add_timer(callback, seconds);
            return;
        }
    }
    callback();
}

void web_ws_relay_action(char * data)
{
    data += 5;
    if (strncmp(data, " on", 3) == 0) {
        // relay_on();
        data += 3;
        web_ws_relay_action_timer(relay_on, data);
    } else if (strncmp(data, " off", 4) == 0) {
        // relay_off();
        data += 4;
        web_ws_relay_action_timer(relay_off, data);        
    } else if (strncmp(data, " toggle", 7) == 0) {
        // relay_toggle();
        data += 7;
        web_ws_relay_action_timer(relay_toggle, data);
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

void web_ws_parse(char *data)
{
    if (strncmp(data, "wifi/set ", 9) == 0) {
        web_ws_set_wifi(data);
    } else if (strncmp(data, "relay", 5) == 0) {
        web_ws_relay_action(data);
    } else if (strncmp(data, "rf/save", 7) == 0) {
        web_ws_rf_save_action(data);
    } else if (strncmp(data, "ota", 3) == 0) {
        web_ws_ota_action(data);
    }
}

void web_send_all(char * msg)
{
    logInfo("Send msg: %s\n", msg);
    web_client_ws_send(msg);
}
