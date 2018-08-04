
#ifndef __WIFI_H__
#define __WIFI_H__

void wifi_init(void);
void wifi_new_connection(char * ssid, char * password);
void wifi_access_point_off(void);
const char * get_uid(void);
void save_uid(char * data);
void wifi_wait_connection(void);
const char * get_mac_uid();

#endif
