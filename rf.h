
#ifndef __RF_H__
#define __RF_H__

#include <espressif/esp_common.h>
#include <queue.h>

#include "config.h"
#include "action.h"
#include "rf_receiver.h"

#define RF_STORE_SIZE 5
#define RF_CODE_SIZE RF_RESULT_SIZE+2 // -0 where o is protocol

void rf_task(void *pvParameters);

#ifdef RF_STORE
struct Store {
    int action;
    int timer;
    int timer_id;
    char code[RF_CODE_SIZE];
};
static const Store store[] = RF_STORE;
#endif

class Rf //: public esp_open_rtos::thread::task_t
{
    private:
        char cmd[1024];
        char last_code[RF_CODE_SIZE];
        unsigned int last_sent;
        #ifdef RF_STORE
        void trigger_action(int action, int timer, int timer_id);
        void trigger_action_timer(Action * object, int action, int timer, int timer_id);
        void trigger(char * code);
        Action * get_relay(int action);
        #endif

    public:
        QueueHandle_t rf_queue;
        void init();
        void test();
        void onReceived(char * result);
        void consumer(char * code);
};

extern Rf rf;

#endif
