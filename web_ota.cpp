#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>
#include <rboot-api.h>

#include "web_client.h"
#include "web.h"
#include "log.h"

int slot;
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
    rboot_config conf;
    conf = rboot_get_config();
    slot = (conf.current_rom + 1) % conf.count;

    logInfo("OTA start (%d)\n", slot);
    if(slot == conf.current_rom) {
        logError("FATAL ERROR: Only one OTA slot is configured!\n");
        // we could send back an error
    } else {
        web_ota_send(0);
    }
}

void web_ota_recv(struct wsMessage * msg)
{
    // printf(".%d %d\n", msg->len, offset);
    current_offset += msg->len;
    web_ota_send(current_offset);
}
