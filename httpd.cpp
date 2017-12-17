#include <lwip/api.h>
#include <string.h>
#include <espressif/esp_common.h>

#include <mbedtls/sha1.h>
#include <mbedtls/base64.h>

#include "wifi.h"
#include "utils.h"
#include "config.h"

#define WS_KEY_IDENTIFIER "Sec-WebSocket-Key: "
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#ifdef UPNP
    #include "upnp.h"
#endif

bool wsOpen = false;

inline int ishex(int x)
{
	return	(x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}
 
int decode(const char *s, char *dec)
{
	char *o;
	const char *end = s + strlen(s);
	int c;
 
	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+') c = ' ';
		else if (c == '%' && (	!ishex(*s++)	||
					!ishex(*s++)	||
					!sscanf(s - 2, "%2x", &c)))
			return -1;
 
		if (dec) *o = c;
	}
 
	return o - dec;
}

char * ws_action(char * data)
{
    wsOpen = true;

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
    int ok = mbedtls_base64_encode(retval_ptr, 30, &olen, sha1sum, 20);

    printf("Key 64: %s\n", retval_ptr);

    static char response[512];
    snprintf(response, sizeof(response), "HTTP/1.1 101 Switching Protocols\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: %s\r\n\r\n"
                , retval_ptr);
    return response;
}

char * parse_request(char *data)
{
    char * response = NULL;

    printf("Received data:\n%s\n", (char*) data);
    if (!strncmp(data, "GET ", 4) || !strncmp(data, "PUT ", 4)) { // is this really necessary?
        char uri[128];
        str_extract(data, '/', ' ', uri);
        printf("uri: %s\n", uri);

        // printf("try try: %d\n", strncmp(data, "GET /ws", 7)); // we could use this instead
        if (strchr(uri, '/')) { // need / at the end no -> ws://192.168.1.107/ws  yes ws://192.168.1.107/ws/
            char action[32];
            char * next = str_extract(uri, 0, '/', action) + 1;
            if (strcmp(action, "ws") == 0) {
                response = ws_action(data);
            }
            #ifdef UPNP
            else if (strcmp(action, "api") == 0) {
                response = upnp_action(next, data);
            }
            #endif
        }
        else if (strchr(uri, '?')) {
            char ssid[32], password[64];
            char * next = str_extract(uri, '=', '&', ssid);
            // printf(":ssid: %s :next: %s\n", ssid, next);
            str_extract(next, '=', '\0', password);
            // printf(":ssid: %s :pwd: %s\n\n", ssid, password);

            char ssid2[32], password2[64];
            decode(ssid, ssid2);
            decode(password, password2);

            printf(":ssid: %s :pwd: %s\n\n", ssid2, password2);

            wifi_new_connection(ssid2, password2);
        }
    }

    return response;
}

char * httpd_get_default_response()
{
    static char buf[512];
    struct sdk_station_config config;
    sdk_wifi_station_get_config(&config);
    snprintf(buf, sizeof(buf),
            "HTTP/1.1 200 OK\r\n"
            "Content-type: text/html\r\n\r\n"
            "\
            <form>\r\n\
                <label>Wifi SSID</label><br />\r\n\
                <input name='ssid' placeholder='SSID' value='%s' /><br /><br />\r\n\
                <label>Wifi password</label><br />\r\n\
                <input name='password' placeholder='password' value='%s' /><br /><br />\r\n\
                <button type='submit'>Save</button><br />\r\n\
            <form>\r\n", config.ssid, config.password);
    return buf;
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

void ws_read(u8_t * data)
{
    int data_offset = 6;
    u8_t *dptr = &data[6];
    u8_t *kptr = &data[2];
    u16_t len = data[1] & 0x7F;

    if (len == 127) {
        return; // ERR_OK
    } else if (len == 126) {
        /* extended length */
        dptr += 2;
        kptr += 2;
        data_offset += 2;
        len = (data[2] << 8) | data[3];
    }

    /* unmask */
    for (int i = 0; i < len; i++) {
        *(dptr++) ^= kptr[i % 4];
    }

    printf("ws read: %s\n", data);
}

void httpd_task(void *pvParameters)
{
    struct netconn *client = NULL;
    struct netconn *nc = netconn_new(NETCONN_TCP);
    if (nc == NULL) {
        printf("Failed to allocate socket\n");
        vTaskDelete(NULL);
    }
    netconn_bind(nc, IP_ADDR_ANY, HTTPD_PORT);
    netconn_listen(nc);
    char * response = NULL;
    while (1) {
        err_t err = netconn_accept(nc, &client);
        if (err == ERR_OK) {
            struct netbuf *nb;
            wsOpen = false;
            while ((err = netconn_recv(client, &nb)) == ERR_OK) {
                void *data = NULL;
                u16_t len;
                if (netbuf_data(nb, &data, &len) == ERR_OK) {
                    // printf("Received data:\n%.*s\n", len, (char*) data);
                    response = parse_request((char *)data);
                    if (!response) {
                        ws_read((u8_t *)data);
                    }                         
                }          
                if (!response) {
                    if (wsOpen) {
                        printf("ws: send default response\n");
                        response = ws_get_response("hello\n");                         
                    } else {
                        printf("httpd: send default response\n");
                        response = httpd_get_default_response();                         
                    }
                }
                netconn_write(client, response, strlen(response), NETCONN_COPY);
                if (!wsOpen) {
                    break;
                }
            }
            netbuf_delete(nb);
        }
        // printf("Closing connection\n");
        netconn_close(client);
        netconn_delete(client);
    }
}