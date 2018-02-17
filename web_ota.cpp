#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>
#include <rboot-api.h>

#include "web_client.h"
#include "web.h"
#include "log.h"

int slot = -1;
unsigned int current_offset;
unsigned int flash_offset;

void web_ota_send(int offset)
{
    char response[20];
    if (slot > -1) {
        sprintf(response, ". ota %d", offset);
        current_offset = offset;
        web_client_ws_send(response);
    } else {
        web_client_ws_send((char *)". ota error OTA was not initialized");
    }
}

void web_ota_start()
{
    rboot_config conf;
    conf = rboot_get_config();
    slot = (conf.current_rom + 1) % conf.count;

    logInfo("OTA start (%d)\n", slot);
    if(slot == conf.current_rom) {
        slot = -1;
        logError("FATAL ERROR: Only one OTA slot is configured!\n");
        web_client_ws_send((char *)". ota error OTA no slot available");
    } else {
        current_offset = 0;
        flash_offset = conf.roms[slot];
        web_ota_send(0);
    }
}

void web_ota_recv(struct wsMessage * msg)
{
    // printf(".%d %d\n", msg->len, offset);
    current_offset += msg->len;
    web_ota_send(current_offset);
}
