
#ifndef __WEB_H__
#define __WEB_H__

#define OPCODE_CLOSE 0x8

struct wsMessage
{
    u8_t * data;
    uint8_t opcode;
    uint8_t isMasked;
    uint32_t len;
};

int web_close(struct tcp_pcb *pcb);
void web_ws_read(struct wsMessage * msg);

#endif
