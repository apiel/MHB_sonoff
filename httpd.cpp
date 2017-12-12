#include <lwip/api.h>
#include <string.h>
#include <espressif/esp_common.h>

#include "wifi.h"
#include "utils.h"
#include "config.h"
#include "sha1.h"
#include "sha1.bis.h"
#include "base64.h"
#include "base64.bis.h"

#define WS_KEY_IDENTIFIER "Sec-WebSocket-Key: "
#define WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#ifdef UPNP
    #include "upnp.h"
#endif

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

char * parse_request(char *data)
{
    char * response = NULL;

    // printf("Received data:\n%s\n", (char*) data);
    if (!strncmp(data, "GET ", 4) || !strncmp(data, "PUT ", 4)) { // is this really necessary?
        char uri[128];
        str_extract(data, '/', ' ', uri);
        printf("uri: %s\n", uri);

        // printf("try try: %d\n", strncmp(data, "GET /ws", 7)); // we could use this instead
        if (strchr(uri, '/')) { // need / at the end no -> ws://192.168.1.107/ws  yes ws://192.168.1.107/ws/
            char action[32];
            char * next = str_extract(uri, 0, '/', action) + 1;
            if (strcmp(action, "ws") == 0) {
                char * key = strstr(data, WS_KEY_IDENTIFIER) + strlen(WS_KEY_IDENTIFIER);
                key[25] = '\0';
                strcat(key, WS_GUID); // fixed key for websocket haching
                printf("search key: %s\n", key);
                sha1nfo s;
                sha1_init(&s);
				sha1_write(&s, key, strlen(key));

                char b64Result[30];
                base64_encode(b64Result, (char *)sha1_result(&s), 20);
                printf("hashed key1: %s\n", b64Result); 

                char b64Result_bis[30];
                base64_encode_bis(20, sha1_result(&s), sizeof(b64Result_bis), b64Result_bis); 
                printf("hashed key2: %s\n", b64Result_bis); 


                // -----

                uint8_t *hash;
                char result[21];
                char b64Result3[30];

                SHA1Context sha;
                int err;
                uint8_t Message_Digest[20];
                err = SHA1Reset(&sha);
                err = SHA1Input(&sha, reinterpret_cast<const uint8_t *>(key), strlen(key));
                err = SHA1Result(&sha, Message_Digest);
                hash = Message_Digest;

                for (int i=0; i<20; ++i) {
                    result[i] = (char)hash[i];
                }
                result[20] = '\0';

                base64_encode(b64Result3, result, 20);
                printf("hashed key3: %s\n", b64Result3); 
                // --                               
          
                printf("let's try websocket\n");
                static char buf[512];
                snprintf(buf, sizeof(buf), "HTTP/1.1 101 Switching Protocols\r\n"
                           "Upgrade: websocket\r\n"
                           "Connection: Upgrade\r\n"
                           "Sec-WebSocket-Accept: %s\r\n"
                           "Sec-WebSocket-Protocol: chat\r\n\r\n", b64Result);
                response = buf;
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
            if ((err = netconn_recv(client, &nb)) == ERR_OK) {
                void *data = NULL;
                u16_t len;
                if (netbuf_data(nb, &data, &len) == ERR_OK) {
                    // printf("Received data:\n%.*s\n", len, (char*) data);
                    response = parse_request((char *)data);                         
                }          
                if (!response) {
                    printf("httpd: send default response\n");
                    response = httpd_get_default_response();                         
                }
                netconn_write(client, response, strlen(response), NETCONN_COPY);
            }
            netbuf_delete(nb);
        }
        // printf("Closing connection\n");
        netconn_close(client);
        netconn_delete(client);
    }
}