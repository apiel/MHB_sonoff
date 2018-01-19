#include <lwip/api.h> // vtask
#include <lwip/debug.h>
#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>

#include "web.h"
#include "log.h"
#include "wifi.h"
#include "utils.h"
#include "relay.h"

int web_close(struct tcp_pcb *pcb)
{
    int err = ERR_OK;
    if (pcb) {
        logDebug("* Web close\n"); 
        tcp_recv(pcb, NULL);
        err = tcp_close(pcb);
    }
    return err;
}

void web_ws_read(struct wsMessage * msg)
{
    msg->opcode = (*msg->data) & 0x0F;
    msg->data += 1;
    msg->isMasked = (*msg->data) & (1<<7);
    msg->len = (*msg->data) & 0x7F;
    msg->data += 1;
    logDebug("* opcode %d len %d ismasked %d\n", msg->opcode, msg->len, msg->isMasked);

    if (msg->len == 126) {
        memcpy(&msg->len, msg->data, sizeof(uint16_t));
        msg->data += sizeof(uint16_t);
    } else if (msg->len == 127) {
        memcpy(&msg->len, msg->data, sizeof(uint64_t));
        msg->data += sizeof(uint64_t);
    }
}

char * web_ws_encode_msg(char * data)
{
  static char buf[512];
  int len = strlen(data);

  int offset = 2;
  buf[0] = 0x81;
  if (len > 125) {
    offset = 4;
    buf[1] = 126;
    buf[2] = len >> 8;
    buf[3] = len;
  } else {
    buf[1] = len;
  }

  memcpy(&buf[offset], data, len);

  return buf;
}

void web_ws_send(struct tcp_pcb *pcb, char *msg)
{
    if (pcb) {
        msg = web_ws_encode_msg(msg);
        err_t err = tcp_write(pcb, msg, strlen(msg), 0);
        LWIP_ASSERT("Something went wrong while tcp write to client", err == ERR_OK);
    }    
}

char * web_ws_set_wifi(char * data)
{
    static char response[4];

    data += 9;

    char ssid[32], password[64];
    char * next = str_extract(data, 0, ' ', ssid);
    str_extract(next, ' ', '\0', password);
    logInfo("# Set wifi ssid: %s password: %s\n\n", ssid, password);

    wifi_new_connection(ssid, password);

    snprintf(response, sizeof(response), "OK\r\n");
    return response;
}

char * web_ws_relay_get_status()
{
    static char response[11];
    int status = relay_status();
    snprintf(response, sizeof(response), ". relay %d\n", status);
    return response;
}

char * web_ws_relay_action(char * data)
{
    data += 5;
    if (strncmp(data, " on", 3) == 0) {
        relay_on();
    } else if (strncmp(data, " off", 4) == 0) {
        relay_off();
    }
    return web_ws_relay_get_status();
}

char * web_ws_parse(char *data)
{
    char * response = NULL;
    if (strncmp(data, "wifi set ", 9) == 0) {
        response = web_ws_set_wifi(data);
    } else if (strncmp(data, "relay", 5) == 0) {
        response = web_ws_relay_action(data);
    }
    if (response) {
        response = web_ws_encode_msg(response);
    }
    return response;
}
