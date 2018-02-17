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

void web_ota_error()
{
    // todo
}

void web_ota_send(int offset)
{
    char response[20];
    sprintf(response, ". ota %d", offset);
    web_client_ws_send(response);
}

void web_ota_start()
{
    rboot_config conf = rboot_get_config();
    slot = (conf.current_rom + 1) % conf.count;

    logInfo("OTA start ([%d] %d -> %d)\n", conf.count, conf.current_rom, slot);
    if(slot == conf.current_rom) {
        slot = -1;
        logError("FATAL ERROR: Only one OTA slot is configured!\n");
        web_client_ws_send((char *)". ota error OTA no slot available");
    } else {
    //     rboot_write_init(conf.roms[slot]);
        current_offset = 0;
        flash_offset = conf.roms[slot];
        web_ota_send(0);
    }
}

void web_ota_recv(struct wsMessage * msg)
{
    // printf(".%d %d\n", msg->len, offset);
    unsigned int next_offset = current_offset + msg->len;
    if (next_offset  > MAX_FIRMWARE_SIZE) {
        web_client_ws_send((char *)". ota error OTA firmware too large");
    } else if (slot > -1) {
        // rboot_write_status rboot_status;
        // if (!rboot_write_flash(&rboot_status, (uint8 *)msg->data, msg->len)) {
        //     web_client_ws_send((char *)". ota error OTA writting error");
        // } else {
        //     web_ota_send(current_offset);
        // }
        unsigned int flash_pos = flash_offset+current_offset;
        // if(flash_pos % SECTOR_SIZE == 0) {
        if ((flash_pos&(SPI_FLASH_SEC_SIZE-1))==0) {
            sdk_spi_flash_erase_sector(flash_pos / SECTOR_SIZE);
        }
        sdk_spi_flash_write(flash_pos, (uint32_t*)msg->data, (uint32_t)msg->len);
        // sdk_spi_flash_write(flash_pos, (uint32_t*)"muter foker", 11);
        current_offset = next_offset;
        web_ota_send(current_offset);
    } else {
        web_client_ws_send((char *)". ota error OTA was not initialized");
    }
}

void web_ota_end()
{
    logInfo("OTA end\n");

    uint32_t length;
    rboot_config conf = rboot_get_config();
    if (!rboot_verify_image(conf.roms[slot], &length, NULL)) {
        logError("OTA verify image invalid");
        web_client_ws_send((char *)". ota error OTA verify image invalid");
    } else {
        // here we could cheksum the firmware
        web_client_ws_send((char *)". ota success");
        logInfo("OTA reboot on slot %d\n", slot);
        // rboot_set_current_rom(slot);
        // sdk_system_restart();
    }
}
