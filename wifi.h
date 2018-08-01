
#ifndef __WIFI_H__
#define __WIFI_H__

class Wifi {
};
void wifi_toggle(void);
void wifi_off(void);
void wifi_init(void);
void wifi_connect(void);
void wifi_new_connection(char * ssid, char * password);
void wifi_access_point(void);
void wifi_access_point_off(void);
const char * get_uid(void);
void save_uid(char * data);
void wifi_wait_connection(void);

#endif
