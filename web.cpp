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
        logDebug("* Web close\n"); 
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
    logDebug("* opcode %d len %d ismasked %d\n", msg->opcode, msg->len, msg->isMasked);

    if (msg->len == 126) {
        memcpy(&msg->len, msg->data, sizeof(uint16_t));
        msg->data += sizeof(uint16_t);
    } else if (msg->len == 127) {
        memcpy(&msg->len, msg->data, sizeof(uint64_t));
        msg->data += sizeof(uint64_t);
    }
}

char * web_ws_encode_msg(char * data)
{
  static char buf[512];
  int len = strlen(data);

  int offset = 2;
  buf[0] = 0x81;
  if (len > 125) {
    offset = 4;
    buf[1] = 126;
    buf[2] = len >> 8;
    buf[3] = len;
  } else {
    buf[1] = len;
  }

  memcpy(&buf[offset], data, len);

  return buf;
}

void web_ws_send(struct tcp_pcb *pcb, char *msg)
{
    if (pcb) {
        msg = web_ws_encode_msg(msg);
        err_t err = tcp_write(pcb, msg, strlen(msg), 0);
        LWIP_ASSERT("Something went wrong while tcp write to client", err == ERR_OK);
    }    
}
