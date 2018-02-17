#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>
#include <rboot-api.h>

#include "web_client.h"
#include "web.h"
#include "log.h"

#define MAX_FIRMWARE_SIZE 0x100000 /*1MB firmware max at the moment */

int slot = -1;
unsigned int current_offset;
unsigned int flash_offset;

void web_ota_send(int offset)
{
    char response[20];
    sprintf(response, ". ota %d", offset);
    web_client_ws_send(response);
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
    if (current_offset > MAX_FIRMWARE_SIZE) {
        web_client_ws_send((char *)". ota error OTA firmware too large");
    } else if (slot > -1) {
        sdk_spi_flash_write(flash_offset+current_offset, (uint32_t*)msg->data, (uint32_t)msg->len);
        web_ota_send(current_offset);
    } else {
        web_client_ws_send((char *)". ota error OTA was not initialized");
    }
}
