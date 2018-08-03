#include <espressif/esp_common.h>
#include <esp8266.h>
#include <string.h>
#include <rboot-api.h>

// in http_buffered_client if (http_reponse.length && tot_http_pdu_rd != http_reponse.length)

extern "C" {
    #include <http_client_ota.h>
}

#include "mqtt.h"

#define vTaskDelayMs(ms) vTaskDelay((ms) / portTICK_PERIOD_MS)

static inline void ota_error_handling(OTA_err err) {
    printf("Error (%d):", err);

    switch(err) {
    case OTA_DNS_LOOKUP_FALLIED:
        printf("DNS lookup has fallied\n");
        break;
    case OTA_SOCKET_ALLOCATION_FALLIED:
        printf("Impossible allocate required socket\n");
        break;
    case OTA_SOCKET_CONNECTION_FALLIED:
        printf("Server unreachable, impossible connect\n");
        break;
    case OTA_SHA_DONT_MATCH:
        printf("Sha256 sum does not fit downloaded sha256\n");
        break;
    case OTA_REQUEST_SEND_FALLIED:
        printf("Impossible send HTTP request\n");
        break;
    case OTA_DOWLOAD_SIZE_NOT_MATCH:
        printf("Dowload size don't match with server declared size\n");
        break;
    case OTA_ONE_SLOT_ONLY:
        printf("rboot has only one slot configured, impossible switch it\n");
        break;
    case OTA_FAIL_SET_NEW_SLOT:
        printf("rboot cannot switch between rom\n");
        break;
    case OTA_IMAGE_VERIFY_FALLIED:
        printf("Dowloaded image binary checsum is fallied\n");
        break;
    case OTA_UPDATE_DONE:
        printf("Ota has completed upgrade process, all ready for system software reset\n");
        break;
    case OTA_HTTP_OK:
        printf("HTTP server has response 200, Ok\n");
        break;
    case OTA_HTTP_NOTFOUND:
        printf("HTTP server has response 404, file not found\n");
        break;
    default:
        printf("Something went wrong with OTA\n");
        break;
    }
}

void sleep()
{
    // for(int s = 50; s > 0; s--) {
    //     sdk_os_delay_us(0XFFFF);
    // }
    taskYIELD();
    vTaskDelay(100);
}

void ota(char * server, char * port, char * path)
{
    // printf("host: %s, port: %s, path: %s\n", server, port, path);
    static ota_info info;
    info.server = server;
    info.port = port;
    info.binary_path = path;

    OTA_err err;
    err = ota_update(&info);
    ota_error_handling(err);
    if(err == OTA_UPDATE_DONE) {
        printf("ota success, reboot...\n");
        sleep();
        printf("ota success, reboot...\n");
        sleep();
        printf("ota success, reboot...\n");
        sleep();
        printf("ota success, reboot...\n");
        sdk_system_restart();
    } else {
        mqtt_send("log", "ota failed");
    }
}

void ota_prepare(void)
{
    rboot_config rboot_config = rboot_get_config();
    uint8_t nextRom = (rboot_config.current_rom+1)%rboot_config.count;
    printf("ROMROM: current %d count %d offset %d\n", rboot_config.current_rom, rboot_config.count, rboot_config.roms[rboot_config.current_rom]);
    printf("ROMROM: next rom %d next offset %d\n", nextRom, rboot_config.roms[nextRom]);

    if (nextRom != 0) { // we have to keep the first rom, the bootloader will copy the next rom at the first position, so most likely nextRom will always be 1
        int end = rboot_config.roms[nextRom] + 350000; // we assume that our firmware is less than 350 000 bytes
        for(int offset = rboot_config.roms[nextRom]; offset < end; offset += SPI_FLASH_SEC_SIZE) {
            if(sdk_spi_flash_erase_sector(offset/SPI_FLASH_SEC_SIZE) != SPI_FLASH_RESULT_OK) {
                printf("\nsdk_spi_flash_erase_sector stop earlier than plan at %d!\n", offset);
                break;
            }
        }
    }

    // for(int test = 310000; test < 0x2000000; test += SPI_FLASH_SEC_SIZE) {
    //     if(sdk_spi_flash_erase_sector(test/SPI_FLASH_SEC_SIZE) != SPI_FLASH_RESULT_OK) {
    //         printf("\n\nstop at %d\n\n", test);
    //         break;
    //     }
    // }
}
