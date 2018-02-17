#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>

#include "web_client.h"
#include "web.h"

int current_offset = 0;
void web_ota_send(int offset)
{
    char response[20];
    sprintf(response, ". ota %d", offset);
    current_offset = offset;
    web_client_ws_send(response);
}

void web_ota_start()
{
    web_ota_send(0);
}

void web_ota_recv(struct wsMessage * msg)
{
    // printf(".%d %d\n", msg->len, offset);
    current_offset += msg->len;
    web_ota_send(current_offset);
}
