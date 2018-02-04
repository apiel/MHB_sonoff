
#ifndef __RF_H__
#define __RF_H__

#include "RCSwitch.h"
#include "task.hpp"
#include "espressif/esp_common.h"

#define RF_STORE_SIZE 5

class task_rf_t: public esp_open_rtos::thread::task_t
{
    private:
        char cmd[1024];
        struct Store {
            int protocol;
            int action;
            char code[RCSWITCH_MAX_CHANGES];
        };
        Store store[RF_STORE_SIZE];
        void task();    
        void trigger_action(int action);
        void trigger(char * code, int protocol);        

    public:
        void init_store();
        void test();
};

void rf_save_store(char * data);

#endif
