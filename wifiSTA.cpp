
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
extern "C" {
    #include <etstimer.h>
    #include <espressif/osapi.h>
    #include <espressif/esp_timer.h>
    #include <espressif/esp_wifi.h>
    #include <espressif/esp_sta.h>
    #include <espressif/esp_misc.h>
    #include <espressif/esp_system.h>
}

#define CONNTRY_IDLE 0
#define CONNTRY_WORKING 1
#define CONNTRY_SUCCESS 2
#define CONNTRY_FAIL 3
//Connection result var
static int connTryStatus=CONNTRY_IDLE;
static ETSTimer resetTimer;

//Temp store for new ap info.
static struct sdk_station_config stconf;

//This routine is ran some time after a connection attempt to an access point. If
//the connect succeeds, this gets the module in STA-only mode.
static void resetTimerCb(void *arg) {
	int x=sdk_wifi_station_get_connect_status();
	if (x==STATION_GOT_IP) {
		//Go to STA mode. This needs a reset, so do that.
		printf("Got IP. Going into STA mode..\n");
		sdk_wifi_set_opmode(1);
		sdk_system_restart();
	} else {
		connTryStatus=CONNTRY_FAIL;
		printf("Connect fail. Not going into STA-only mode.\n");
		//Maybe also pass this through on the webpage?
	}
}

//Actually connect to a station. This routine is timed because I had problems
//with immediate connections earlier. It probably was something else that caused it,
//but I can't be arsed to put the code back :P
static void reassTimerCb(void *arg) {
	int x;
	printf("Try to connect to AP....\n");
	sdk_wifi_station_disconnect();
	sdk_wifi_station_connect();
	x=sdk_wifi_get_opmode();
	connTryStatus=CONNTRY_WORKING;
	if (x!=1) {
		//Schedule disconnect/connect
		sdk_os_timer_disarm(&resetTimer);
		sdk_os_timer_setfn(&resetTimer, resetTimerCb, NULL);
		sdk_os_timer_arm(&resetTimer, 15000, 0); //time out after 15 secs of trying to connect
	}
}

void wifi_sta_connect()
{
	static ETSTimer reassTimer;
    sdk_wifi_set_opmode(STATION_MODE);

	//Schedule disconnect/connect
	sdk_os_timer_disarm(&reassTimer);
	sdk_os_timer_setfn(&reassTimer, reassTimerCb, NULL);
	sdk_os_timer_arm(&reassTimer, 500, 0);
}

void wifi_sta_new_connection(char * essid, char * passwd)
{
	strncpy((char*)stconf.ssid, essid, 32);
	strncpy((char*)stconf.password, passwd, 64);
	printf("Try to connect to AP %s pw %s\n", essid, passwd);
	
	sdk_wifi_station_set_config(&stconf);

	wifi_sta_connect();
}
