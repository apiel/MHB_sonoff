#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>
#include <rboot-api.h>

#include "web_client.h"
#include "web.h"
#include "log.h"
#include "EEPROM.h"

#define MAX_FIRMWARE_SIZE 0x100000 /*1MB firmware max at the moment */

int slot = -1;
unsigned int current_offset;
unsigned int flash_offset;

void web_ota_saved()
{
    char response[26];
    sprintf(response, ". ota saved %d", current_offset);
    web_client_ws_send(response);
}

void web_ota_next()
{
    char response[25];
    sprintf(response, ". ota next %d", current_offset);
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
        flash_offset = conf.roms[slot];
        current_offset = 0;
        web_ota_next();
    }
}

// we might create a file flash
// maybe with EEPROMClass::save
void web_ota_flash(uint32_t *chunk, uint16_t chunk_len)
{
    int offset = 0;
    // if (current_offset == 0 && flash_offset % SECTOR_SIZE == 0) {
    //     sdk_spi_flash_erase_sector(flash_offset / SECTOR_SIZE);
    // }
    if(chunk_len && ((uint32_t)chunk % 4)) {
        // sdk_spi_flash_write requires a word aligned
        // Assuming chunk_len is always a multiple of 4 bytes.
        uint32_t first_word;
        memcpy(&first_word, chunk, 4);
        sdk_spi_flash_write(flash_offset + current_offset, &first_word, 4);
        memmove(LWIP_MEM_ALIGN(chunk),&chunk[1],chunk_len-4);
        chunk = (uint32_t *)LWIP_MEM_ALIGN(chunk);
        offset += 4;
        chunk_len -= 4;
    }
    sdk_spi_flash_write(flash_offset + current_offset + offset, chunk, chunk_len);
}

void web_ota_recv(struct wsMessage * msg)
{
    if (current_offset + msg->len  > MAX_FIRMWARE_SIZE) {
        logError("FATAL ERROR: OTA firmware too large\n");
        web_client_ws_send((char *)". ota error OTA firmware too large");
    } else if (slot > -1) {
        if (msg->len < 126) printf("there might be a pb because len < 126");
        // msg->data32 += msg->len > 125 ? 4 : 2;
        msg->data32++; // but this is right only if len > 125, need to find a fix
        printf(".");    
        web_ota_flash(msg->data32, msg->len);

        web_ota_saved();
        current_offset += msg->len;
    } else {
        logError("FATAL ERROR: OTA was not initialized\n");
        web_client_ws_send((char *)". ota error OTA was not initialized");
    }
}

void web_ota_end()
{
    logInfo("OTA end\n");
    web_client_ws_send((char *)". ota success");

    rboot_config conf = rboot_get_config();
    uint32_t length;
    if (!rboot_verify_image(conf.roms[slot], &length, NULL)) {
        logError("OTA verify image invalid");
        web_client_ws_send((char *)". ota error OTA verify image invalid");
    } else {
        // here we could cheksum the firmware
        web_client_ws_send((char *)". ota success");
        logInfo("OTA reboot on slot %d\n", slot);
        rboot_set_current_rom(slot);
        sdk_system_restart();
    }
}

void web_ota_get_slot() {
    char response[22];
    sprintf(response, ". ota current slot %d", rboot_get_current_rom());
    web_client_ws_send(response);
}

void web_ota_set_slot(char n) {
    rboot_set_current_rom(n - '0');
    sdk_system_restart();
}
