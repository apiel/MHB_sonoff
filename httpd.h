
#ifndef __HTTPD_H__
#define __HTTPD_H__

void ws_send_queue(char *msg);
void httpd_task(void *pvParameters);

#endif
