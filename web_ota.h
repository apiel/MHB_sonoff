
#ifndef __WEB_OTA_H__
#define __WEB_OTA_H__

void web_ota_start();
void web_ota_end();
void web_ota_recv(struct wsMessage * msg);
void web_ota_next();
void web_ota_get_slot();
void web_ota_set_slot(char n);

#endif
