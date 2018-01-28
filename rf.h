
#ifndef __RF_H__
#define __RF_H__

#include "RCSwitch.h"
#include "task.hpp"
#include "espressif/esp_common.h"

#define RF_STORE_SIZE 5

class task_rf_t: public esp_open_rtos::thread::task_t
{
    private:
        struct Store {
            int protocol;
            int action;
            char code[RCSWITCH_MAX_CHANGES];
        };
        Store store[RF_STORE_SIZE];
        void task();    

    public:
        void init_store();
};

void rf_save_store(char * data);

#endif
