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

#define WS_KEY_IDENTIFIER "Sec-WebSocket-Key: "
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define OPCODE_CLOSE 0x8

#ifdef UPNP
    #include "upnp.h"
#endif

struct http_state {
  u8_t is_websocket;
};

char * ws_action(char * data)
{
    char * key = strstr(data, WS_KEY_IDENTIFIER) + strlen(WS_KEY_IDENTIFIER);
    key[24] = '\0';
    strcat(key, WS_GUID);
    printf("Resulting key: %s\n", key);

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

    printf("Key 64: %s\n", retval_ptr);

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
    function wifi() {\r\n\
        const ssid = document.getElementById('ssid').value;\r\n\
        const password = document.getElementById('password').value;\r\n\
        const s = new WebSocket(`ws://${window.location.hostname}/ws/`);\r\n\
        s.onopen = () => { s.send(`wifi set ${ssid} ${password}`); s.close(); };\r\n\
    }\r\n\
    </script>\r\n\
    <label>Wifi SSID</label><br>\r\n\
    <input id='ssid' placeholder='SSID' value='%s'><br><br>\r\n\
    <label>Wifi password</label><br>\r\n\
    <input id='password' placeholder='password' value='%s'><br><br>\r\n\
    <button onclick='wifi()'>Save</button><br>\r\n", config.ssid, config.password);
    return buf;
}

char * parse_request(char *data, struct http_state *hs)
{
    char * response = NULL;

    // printf("Received data (ws %d):\n%s\n", hs->is_websocket, (char*) data);
    if (strncmp(data, "GET /ws", 7) == 0) {
        response = ws_action(data);
        hs->is_websocket = 1;
    } else {
        printf("Httpd: send default response\n");
        response = httpd_get_default_response();            
    }

    // need to set wifi with websocket or with post
    // Post: else if (strncmp(data, "POST /wifi", 10) == 0)
    // else if (strchr(uri, '?')) {
    //     char ssid[32], password[64];
    //     char * next = str_extract(uri, '=', '&', ssid);
    //     // printf(":ssid: %s :next: %s\n", ssid, next);
    //     str_extract(next, '=', '\0', password);
    //     // printf(":ssid: %s :pwd: %s\n\n", ssid, password);

    //     char ssid2[32], password2[64];
    //     decode(ssid, ssid2);
    //     decode(password, password2);

    //     printf(":ssid: %s :pwd: %s\n\n", ssid2, password2);

    //     wifi_new_connection(ssid2, password2);
    // }    

    return response;
}

char * ws_get_response(char * data)
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

void http_close(struct tcp_pcb *pcb)
{
    printf("Close connection.\n");
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
}

void ws_read(u8_t * data, struct tcp_pcb *pcb, struct http_state *hs)
{
    uint8_t opcode = (*data) & 0x0F;
    if (opcode == OPCODE_CLOSE) {
        hs->is_websocket = 0;
        http_close(pcb);
    } else {
        data += 1;
        uint8_t isMasked = (*data) & (1<<7);
        uint32_t len = (*data) & 0x7F;
        data += 1;
        printf("opcode %d len %d ismasked %d\n", opcode, len, isMasked);

        if (len == 126) {
            memcpy(&len, data, sizeof(uint16_t));
            data += sizeof(uint16_t);
        } else if (len == 127) {
            memcpy(&len, data, sizeof(uint64_t));
            data += sizeof(uint64_t);
        }

        if (isMasked) {
            uint32_t maskingKey;
            memcpy(&maskingKey, data, sizeof(uint32_t));
            data += sizeof(uint32_t);
            for (uint32_t i = 0; i < len; i++) {
                int j = i % 4;
                data[i] = data[i] ^ ((uint8_t *)&maskingKey)[j];
            }
            printf("ws data: %s\n", data);
        } else {
            printf("ws we should close connexion... masked...\n");
        }
    }
}

static err_t http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct http_state *hs = (struct http_state *)arg;

    u8_t *data = (u8_t *) p->payload;

    // printf("rcv data (ws %d): %s\n", hs->is_websocket, data);

    char * response = NULL;
    if (hs->is_websocket) {
        ws_read((u8_t *)data, pcb, hs);
        // response = ws_get_response("hello\n");  
    } else {
        response = parse_request((char *)data, hs);
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
        http_close(pcb);   
    }
    return ERR_OK;
}

static err_t http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    struct tcp_pcb_listen *lpcb = (struct tcp_pcb_listen*)arg;
    printf("http_accept %p / %p\n", (void*)pcb, arg); 

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
void httpd_task(void *pvParameters)
{
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

    while(1) {
        vTaskDelay(1000);
    }
}