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
    err_t err = ERR_OK;
    if (ws_pcb_c) {
        logInfo("WS close\n"); 
        tcp_recv(ws_pcb_c, NULL);
        err = tcp_close(ws_pcb_c);
        ws_pcb_c = NULL; 
    }
    ws_is_connected = false;
    return err;
}

char * ws_read2(u8_t * data, struct tcp_pcb *pcb)
{
    printf("ws_read2\n");
    char * response = NULL;
    uint8_t opcode = (*data) & 0x0F;
    if (opcode == OPCODE_CLOSE) { // WSop_text = 0x01,         ///< %x1 denotes a text frame
        printf("ws close opcode\n");
    } else {
        data += 1;
        uint8_t isMasked = (*data) & (1<<7);
        uint32_t len = (*data) & 0x7F;
        data += 1;
        logDebug("opcode %d len %d ismasked %d\n", opcode, len, isMasked);

        if (len == 126) {
            memcpy(&len, data, sizeof(uint16_t));
            data += sizeof(uint16_t);
        } else if (len == 127) {
            memcpy(&len, data, sizeof(uint64_t));
            data += sizeof(uint64_t);
        }

        printf("ws_read2 a: %s\n", data);
        if (isMasked) {
            uint32_t maskingKey;
            memcpy(&maskingKey, data, sizeof(uint32_t));
            data += sizeof(uint32_t);
            for (uint32_t i = 0; i < len; i++) {
                int j = i % 4;
                data[i] = data[i] ^ ((uint8_t *)&maskingKey)[j];
            }
            data[len] = '\0';
            printf("ws_read2 b: %s\n", data);
            // response = ws_parse((char *)data);
        } else {
            logInfo("ws we should close connexion... masked...\n");
        }
    }
    return response;
}

static err_t ws_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    // err_t err;
    if (p == NULL) {
        ws_close();
    } else {
        u8_t *data = (u8_t *) p->payload;
        printf("ws_c data: %s\n", data);    
        ws_read2(data, pcb);   
    }
    pbuf_free(p);
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
                "Sec-WebSocket-Version: 13\r\n"
                "Sec-WebSocket-Key: a0YBiKi7u7cdhbz8xu5FWQ==\r\n\r\n");

    err = tcp_write(ws_pcb_c, response, strlen(response), 0);
    printf("some info after write: %i\n", err);

    return ERR_OK;
}

void web_client_task(void *pvParameters)
{
    err_t err;

    ip_addr_t remote_addr;
    IP4_ADDR(&remote_addr, 192, 168, 1, 106);

    while(1) {
        printf("loop %d %d\n", ws_pcb_c != NULL, ws_is_connected);
        if (!ws_is_connected) {
            ws_close();
        }
        if (!ws_pcb_c) {
            printf("try to conect ws\n");
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
