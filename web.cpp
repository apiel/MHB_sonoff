#include <lwip/api.h> // vtask
#include <lwip/debug.h>
#include <lwip/tcp.h>
#include <string.h>
#include <espressif/esp_common.h>

#include "web.h"
#include "log.h"

int web_close(struct tcp_pcb *pcb)
{
    int err = ERR_OK;
    if (pcb) {
        logInfo("Web close\n"); 
        tcp_recv(pcb, NULL);
        err = tcp_close(pcb);
    }
    return err;
}

void web_ws_read(struct wsMessage * msg)
{
    msg->opcode = (*msg->data) & 0x0F;
    msg->data += 1;
    msg->isMasked = (*msg->data) & (1<<7);
    msg->len = (*msg->data) & 0x7F;
    msg->data += 1;
    logDebug("opcode %d len %d ismasked %d\n", msg->opcode, msg->len, msg->isMasked);

    if (msg->len == 126) {
        memcpy(&msg->len, msg->data, sizeof(uint16_t));
        msg->data += sizeof(uint16_t);
    } else if (msg->len == 127) {
        memcpy(&msg->len, msg->data, sizeof(uint64_t));
        msg->data += sizeof(uint64_t);
    }
}
