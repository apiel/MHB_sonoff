
#ifndef __HTTPD_H__
#define __HTTPD_H__

void ws_send(char *msg);
void httpd_task(void *pvParameters);
void ws_task(void *pvParameters);

#endif
