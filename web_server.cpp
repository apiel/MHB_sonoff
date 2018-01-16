#include <lwip/api.h> // vtask
#include <lwip/debug.h>
#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>

#include <mbedtls/sha1.h>
#include <mbedtls/base64.h>

#include "wifi.h"
#include "utils.h"
#include "config.h"
#include "web.h"
#include "log.h"
#include "version.h"

#define WS_KEY_IDENTIFIER "Sec-WebSocket-Key: "
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

struct tcp_pcb * ws_pcb = NULL;

struct http_state {
  u8_t is_websocket;
};

char * ws_encode_response(char * data)
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

void web_server_ws_send(char *msg)
{
    if (ws_pcb) {
        msg = ws_encode_response(msg);
        err_t err = tcp_write(ws_pcb, msg, strlen(msg), 0);
        LWIP_ASSERT("Something went wrong while tcp write from queue", err == ERR_OK);
    }    
}

char * httpd_ws_action(char * data)
{
    char * key = strstr(data, WS_KEY_IDENTIFIER) + strlen(WS_KEY_IDENTIFIER);
    key[24] = '\0';
    strcat(key, WS_GUID);
    logDebug("Resulting key: %s\n", key);

    /* Get SHA1 */
    unsigned char sha1sum[20];
    mbedtls_sha1((unsigned char *) key, strlen(key), sha1sum);

    /* Base64 encode */
    unsigned char retval[100];
    unsigned char *retval_ptr;
    retval_ptr = retval;                
    unsigned int olen;
    mbedtls_base64_encode(NULL, 0, &olen, sha1sum, 20); //get length
    mbedtls_base64_encode(retval_ptr, 30, &olen, sha1sum, 20);

    logInfo("Send websocket response with key: %s\n", retval_ptr);

    static char response[512];
    snprintf(response, sizeof(response), "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: %s\r\n\r\n"
                , retval_ptr);

    return response;
}

char * httpd_get_default_response()
{
    static char buf[1024];
    struct sdk_station_config config;
    sdk_wifi_station_get_config(&config);
    snprintf(buf, sizeof(buf),
            "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "\
    <script>\r\n\
    const s = new WebSocket(`ws://${window.location.hostname}/ws/`);\r\n\
    s.onmessage = (event) => { console.log('recv:', event.data); };\r\n\
    function wifi() {\r\n\
        const ssid = document.getElementById('ssid').value;\r\n\
        const password = document.getElementById('password').value;\r\n\
        s.send(`wifi set ${ssid} ${password}`);\r\n\
    }\r\n\
    </script>\r\n\
    <p>Compile version: %s</p>\r\n\
    <label>Wifi SSID</label><br>\r\n\
    <input id='ssid' placeholder='SSID' value='%s'><br><br>\r\n\
    <label>Wifi password</label><br>\r\n\
    <input id='password' placeholder='password' value='%s'><br><br>\r\n\
    <button onclick='wifi()'>Save</button><br>\r\n", VERSION, config.ssid, config.password);
    return buf;
}

char * parse_request(char *data, struct tcp_pcb *pcb, struct http_state *hs)
{
    char * response = NULL;
    if (strncmp(data, "GET /ws", 7) == 0) {
        response = httpd_ws_action(data);
        hs->is_websocket = 1;
        ws_pcb = pcb; 
    } else {
        logInfo("Httpd: send default response\n");
        response = httpd_get_default_response();            
    }
    return response;
}

static err_t http_close(struct tcp_pcb *pcb, struct http_state *hs) 
{
    if (hs->is_websocket) {
        hs->is_websocket = 0;
        ws_pcb = NULL;
    }
    return web_close(pcb);
}

char * ws_set_wifi(char * data)
{
    static char response[4];

    data += 9;

    char ssid[32], password[64];
    char * next = str_extract(data, 0, ' ', ssid);
    str_extract(next, ' ', '\0', password);
    logInfo("Set wifi ssid: %s password: %s\n\n", ssid, password);

    wifi_new_connection(ssid, password);

    snprintf(response, sizeof(response), "OK\r\n");
    return response;
}

char * ws_parse(char *data)
{
    char * response = NULL;
    if (strncmp(data, "wifi set ", 9) == 0) {
        response = ws_set_wifi(data);
    }
    if (response) {
        response = ws_encode_response(response);
    }
    return response;
}

char * ws_read(u8_t * data, struct tcp_pcb *pcb, struct http_state *hs)
{
    char * response = NULL;
    struct wsMessage msg;
    msg.data = data;
    web_ws_read(&msg);

    if (msg.opcode == OPCODE_CLOSE) {
        http_close(pcb, hs);
    } else {
        if (msg.isMasked) {
            uint32_t maskingKey;
            memcpy(&maskingKey, msg.data, sizeof(uint32_t));
            msg.data += sizeof(uint32_t);
            for (uint32_t i = 0; i < msg.len; i++) {
                int j = i % 4;
                msg.data[i] = msg.data[i] ^ ((uint8_t *)&maskingKey)[j];
            }
            msg.data[msg.len] = '\0';
            response = ws_parse((char *)msg.data);
        } else {
            logInfo("ws we should close connexion... masked...\n");
        }
    }
    return response;
}

static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct http_state *hs = (struct http_state *)arg;
    u8_t *data = (u8_t *) p->payload;
    char * response = NULL;
    if (p == NULL) {
        http_close(pcb, hs);
    }
    else if (hs->is_websocket) {
        ws_read((u8_t *)data, pcb, hs);
    } else {
        response = parse_request((char *)data, pcb, hs);
    }
    if (response) {
        err_t err = tcp_write(pcb, response, strlen(response), 0);
        LWIP_ASSERT("Something went wrong while tcp write", err == ERR_OK);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    if (!hs->is_websocket) {
        http_close(pcb, hs);   
    }
    return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;
    logDebug("http_accept %p / %p\n", (void*)pcb, arg); 

    /* Decrease the listen backlog counter */
    tcp_accepted(lpcb);
    /* Set priority */
    tcp_setprio(pcb, TCP_PRIO_MIN);    
 
    /* Set up the various callback functions */
    tcp_recv(pcb, http_recv);
    tcp_sent(pcb, http_sent);

    return ERR_OK;
}

// maybe we dont need to create a task
void web_server_task(void *pvParameters)
{
#ifdef HTTPD_PORT
    struct tcp_pcb *pcb;
    err_t err;

    pcb = tcp_new();
    LWIP_ASSERT("httpd_init: tcp_new failed", pcb != NULL);
    tcp_setprio(pcb, TCP_PRIO_MIN);
    // /* set SOF_REUSEADDR here to explicitly bind httpd to multiple interfaces */
    err = tcp_bind(pcb, IP_ADDR_ANY, HTTPD_PORT);
    LWIP_ASSERT("httpd_init: tcp_bind failed", err == ERR_OK);
    pcb = tcp_listen(pcb);
    LWIP_ASSERT("httpd_init: tcp_listen failed", pcb != NULL);
    // /* initialize callback arg and accept callback */
    tcp_arg(pcb, pcb);
    tcp_accept(pcb, http_accept);
#endif

    while(1) {
        vTaskDelay(1000);
    }
}
