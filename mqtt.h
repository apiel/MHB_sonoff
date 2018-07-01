#ifndef _MQTT_h
#define _MQTT_h

void mqtt_start();
void mqtt_send(const char * topic, const char * payload);

#endif
