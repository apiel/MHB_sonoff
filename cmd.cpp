
#include <espressif/esp_common.h>

#include "web_server.h"
#include "web_client.h"

void cmd_send(char * cmd) 
{
    printf(cmd);
    web_client_ws_send(cmd);
    web_server_ws_send(cmd);
}
