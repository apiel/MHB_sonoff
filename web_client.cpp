#include <lwip/api.h> // vtask
#include <lwip/debug.h>
#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>

#include "wifi.h"
#include "web.h"
#include "utils.h"
#include "config.h"
#include "log.h"

struct tcp_pcb * ws_pcb_c = NULL;
bool ws_is_connected = false;

static err_t ws_close() 
{
    err_t err = web_close(ws_pcb_c);
    ws_pcb_c = NULL;
    ws_is_connected = false;
    return err;
}

char * ws_read(u8_t * data)
{
    struct wsMessage msg;
    msg.data = data;
    web_ws_read(&msg);

    // printf("ws_read a: %s\n", msg.data);
    // printf("opcode %d len %d ismasked %d\n", msg.opcode, msg.len, msg.isMasked);

    return NULL;
}

static err_t ws_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    // err_t err;
    if (p == NULL) {
        ws_close();
    } else {
        u8_t *data = (u8_t *) p->payload;
        printf("ws_c data: %s\n", data);    
        ws_read(data);   
    }
    pbuf_free(p);
    return ERR_OK;
}

void web_client_ws_send(char *msg) {
    web_ws_send(ws_pcb_c, msg); 
}

/** TCP connected callback (active connection), send data now */
static err_t ws_tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    logDebug("* WS connected\n");  
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

    return err;
}

void web_client_task(void *pvParameters)
{
    err_t err;

    ip_addr_t remote_addr;
    // IP4_ADDR(&remote_addr, 192, 168, 1, 109);
    IP4_ADDR(&remote_addr, 192, 168, 1, 111);

    while(1) {
        // printf("loop %d %d\n", ws_pcb_c != NULL, ws_is_connected);
        if (!ws_is_connected) {
            ws_close();
        }
        if (!ws_pcb_c) {
            logDebug("* WS try to connect\n");
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
