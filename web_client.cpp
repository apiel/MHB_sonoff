#include <lwip/api.h> // vtask
#include <lwip/debug.h>
#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>

#include "wifi.h"
#include "web.h"
#include "web_client.h"
#include "web_ota.h"
#include "utils.h"
#include "config.h"
#include "log.h"
#include "controller.h"

#define POLL_INTERVAL 4

unsigned int retries = 0;

struct tcp_pcb * ws_pcb_c = NULL;
bool ws_is_connected = false;

bool web_client_ws_is_connected()
{
    return ws_is_connected;
}

static err_t ws_close() 
{
    err_t err = web_close(ws_pcb_c);
    ws_pcb_c = NULL;
    ws_is_connected = false;
    return err;
}

void web_client_read(void * data)
{
    struct wsMessage msg;
    retries = 0;
    msg.data = (uint8_t *)data;
    msg.data32 = (uint32_t *)data;
    web_ws_read(&msg);

    // printf("ws_read []: %s\n", msg.opcode, msg.data);
    // printf("opcode %d len %d ismasked %d\n", msg.opcode, msg.len, msg.isMasked);

    if (msg.opcode == OPCODE_BINARY) {
        web_ota_recv(&msg);
    } else {
        controller_parse((char *)msg.data);
    }
}

static err_t ws_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    // err_t err;
    if (p == NULL) {
        ws_close();
    } else {
        tcp_recved(pcb, p->tot_len);
        void *data = p->payload;
        web_client_read(data);   
    }
    pbuf_free(p);
    return ERR_OK;
}

void web_client_ws_send(char *msg) {
    web_ws_send(ws_pcb_c, msg); 
}

void web_client_ping() {
    web_ws_send(ws_pcb_c, (char *)"ping", OPCODE_PING); 
}

static err_t web_client_poll(void *arg, struct tcp_pcb *pcb)
{
    retries++;
    if (retries > 5) {
        logInfo("WS timeout\n");  
        ws_close();
    } else {
        // logDebug("WS ping\n");  
        web_client_ping();
    }
    return ERR_OK;
}

/** TCP connected callback (active connection), send data now */
static err_t ws_tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    logDebug("WS connected\n");  
    ws_is_connected = true;
    tcp_recv(ws_pcb_c, ws_recv);

    static char response[512];
    snprintf(response, sizeof(response), 
                "GET / HTTP/1.1\r\n"
                "Host: 127.0.0.1:8080\r\n"
                "Connection: Upgrade\r\n"
                "Upgrade: websocket\r\n"
                "Device: %s\r\n"
                "Sec-WebSocket-Version: 13\r\n"
                "Sec-WebSocket-Key: a0YBiKi7u7cdhbz8xu5FWQ==\r\n\r\n", get_uid());

    err = tcp_write(ws_pcb_c, response, strlen(response), 0);

// we might put ping / pong system
// we might have timeout
// tcp_poll
// tcp_err
    tcp_poll(pcb, web_client_poll, POLL_INTERVAL);

    return err;
}

void web_client_set_ip(ip_addr_t * remote_addr)
{
    if (sdk_wifi_get_opmode() == STATION_MODE) {
        // IP4_ADDR(remote_addr, 192, 168, 1, 111);
        IP4_ADDR(remote_addr, 51, 254, 38, 158);
        // IP4_ADDR(remote_addr, 192, 168, 10, 101);
    } else {
        IP4_ADDR(remote_addr, 192, 168, 0, 2);
    }
}

void web_client_task(void *pvParameters)
{
    err_t err;

    ip_addr_t remote_addr;

    while(1) {
        // we might to re-think this loop
        // logDebug("loop %d %d\n", ws_pcb_c != NULL, ws_is_connected);
        if (!ws_is_connected) {
            ws_close();
        }
        if (ws_pcb_c && sdk_wifi_get_opmode() == STATION_MODE && sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
            // printf("ws we should close %d == %d && %d != %d\n", sdk_wifi_get_opmode(), STATION_MODE, sdk_wifi_station_get_connect_status(), STATION_GOT_IP);
            ws_close();
        }
        if (!ws_pcb_c && (sdk_wifi_get_opmode() == SOFTAP_MODE || sdk_wifi_station_get_connect_status() == STATION_GOT_IP)) {
            web_client_set_ip(&remote_addr);
            logDebug("WS try to connect\n");
            ws_pcb_c = tcp_new();
            LWIP_ASSERT("httpd_init: tcp_new failed", ws_pcb_c != NULL);

            #ifdef WS_CLIENT_PORT
            err = tcp_connect(ws_pcb_c, &remote_addr, WS_CLIENT_PORT, ws_tcp_client_connected);
            #endif
            LWIP_ASSERT("ws_init: tcp_connect failed", err == ERR_OK);
        }
        vTaskDelay(300);
    }
}
