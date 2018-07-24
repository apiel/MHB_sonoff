
#ifndef __RF_H__
#define __RF_H__

#include <RCSwitch.h>
#include <espressif/esp_common.h>

#include "action.h"

#define RF_STORE_SIZE 5

class Rf //: public esp_open_rtos::thread::task_t
{
    private:
        char cmd[1024];
        struct Store {
            int protocol;
            int action;
            int timer;
            int timer_id;
            char code[RCSWITCH_MAX_CHANGES];
        };
        Store store[RF_STORE_SIZE];
        void trigger_action(int action, int timer, int timer_id);
        void trigger_action_timer(Action * object, int action, int timer, int timer_id);
        void trigger(char * code, int protocol);
        Action * get_relay(int action);
        void init_store();

    public:
        void init();
        void test();
        void onReceived(char * result);
};

void rf_save_store(char * data);

extern Rf rf;

#endif
