
#ifndef __WEB_H__
#define __WEB_H__

#include "relay.h"

#define OPCODE_CONTINUE 0x0
#define OPCODE_TEXT 0x1
#define OPCODE_BINARY 0x2
#define OPCODE_CLOSE 0x8
#define OPCODE_PING 0x9
#define OPCODE_PONG 0xA

struct wsMessage
{
    uint8_t * data;
    uint32_t * data32;
    uint8_t opcode;
    uint8_t isMasked;
    uint32_t len;
};

int web_close(struct tcp_pcb *pcb);
void web_ws_read(struct wsMessage * msg);
// char * web_ws_encode_msg(char * data);
char * web_ws_encode_msg(char * data, unsigned int opcode = OPCODE_TEXT);
void web_ws_send(struct tcp_pcb *pcb, char *msg, unsigned int opcode = OPCODE_TEXT);
void web_ws_parse(char *data);
void web_send_all(char * msg);
void web_ws_relay_send_status(Relay * relay);
void web_ws_temperature_send(int16_t temperature);
void web_ws_humidity_send(int16_t humidity);

#endif
