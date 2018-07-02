#include <espressif/esp_common.h>
#include <esp8266.h>
#include <string.h>

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


void ota(char * server, char * port, char * path)
{
    // printf("host: %s, port: %s, path: %s\n", server, port, path);
    ota_info info;
    info.server = server;
    info.port = port;
    info.binary_path = path;

    OTA_err err;
    err = ota_update(&info);
    ota_error_handling(err);
    if(err == OTA_UPDATE_DONE) {
        mqtt_send("log", "ota success, reboot in 1sec...");
        sdk_os_delay_us(1000);
        printf("Reset\n");
        sdk_system_restart();
    } else {
        mqtt_send("log", "ota failed");
    }
}
