
#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include "relay.h"

void controller_parse(char *data);
void controller_relay_send_status(Relay * relay);
void controller_temperature_send(int16_t temperature);
void controller_humidity_send(int16_t humidity);

#endif
