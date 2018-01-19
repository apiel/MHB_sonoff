
#ifndef __RF_H__
#define __RF_H__

#include "task.hpp"
#include "espressif/esp_common.h"

class task_rf_t: public esp_open_rtos::thread::task_t
{
    private:
        void task();    
};

#endif
