
#ifndef __WEB_CLIENT_H__
#define __WEB_CLIENT_H__

void web_client_task(void *pvParameters);
void web_client_ws_send(char *msg);
void web_client_start_ota();
bool web_client_ws_is_connected();

#endif
