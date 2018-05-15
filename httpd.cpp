#include <lwip/api.h>
#include <string.h>
#include <espressif/esp_common.h>

// #include "action.h"
#include "utils.h"
// #include "config.h"

#include "upnp.h"

// need refactoring and cleanup

char * parse_request(void *data)
{
    char * response = NULL;
    char uri[128];
    str_extract((char *)data, '/', ' ', uri);
    printf("uri: %s\n", uri);

    if (strchr(uri, '/')) {
        char action[32];
        char * next = str_extract(uri, 0, '/', action) + 1;
        if (strcmp(action, "api") == 0) {
            response = upnp_action(next, (char *)data);
        }
    }

    return response;
}

#define HTTPD_PORT 80

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
                    response = parse_request(data);
                }
                if (!response) {
                    response = (char *)"HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\nHello\r\n";
                }
                netconn_write(client, response, strlen(response), NETCONN_COPY);
            }
            netbuf_delete(nb);
        }
        netconn_close(client);
        netconn_delete(client);
    }
}