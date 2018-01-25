#include "espressif/esp_common.h"
#include "FreeRTOS.h"

extern "C" {
    #include "esp_spiffs.h"
}

void store_init()
{
#if SPIFFS_SINGLETON == 1
    esp_spiffs_init();
#else
    // for run-time configuration when SPIFFS_SINGLETON = 0
    esp_spiffs_init(0x210000, 0x10000);
#endif

    if (esp_spiffs_mount() != SPIFFS_OK) {
        printf("Error mount SPIFFS\n");
    }
}
