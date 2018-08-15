#include <FreeRTOS.h>
#include <queue.h>
#include <string.h>
#include <task.hpp>

#include "rf.h"
#include "config.h"
#include "relay.h"
#include "timer.h"
#include "action.h"
#include "rf_receiver.h"

Rf rf = Rf();

void rf_task(void *pvParameters)
{
    char code[RF_CODE_SIZE];
    rf.init();
    while(1){
        while(xQueueReceive(rf.rf_queue, &code, 0) == pdTRUE){
            // printf("Got code from queue: %s\n", code);
            rf.consumer(code);
        }
        taskYIELD();
        vTaskDelay(10);
    }
}

void Rf::onReceived(char * result) {
    printf("Result::: %s\n", result);
    if (rf_queue && xQueueSend(rf_queue, result, 0) == pdFALSE) {
        printf("Rf queue overflow (no more place in queue).\r\n");
    }
}

void Rf::consumer(char * result) {
    // printf("Consume queue: %s\n", result);
    // here we could send the rf msg received
    // we could send udp multicast messages
    // if (sdk_system_get_time() - last_sent > 1000000 && strcmp(last_code, result) == 0) { // 1sec
    //     mqtt_send("rf/recv", (const char *)result);
    //     last_sent = sdk_system_get_time();
    // }
    trigger(result);
    strcpy(last_code, result);
}

void Rf::init()
{
    rf_queue = xQueueCreate(3, RF_CODE_SIZE);
    init_store();
    printf("Start rf receiver\n");
    rfReceiver.start(PIN_RF433_RECEIVER, [](char * result){
        rf.onReceived(result);
    });
}

// void rf_save_store(char * data)
// {
//     EEPROM.save(EEPROM_RF_STORE_START, (uint8_t *)data, strlen(data));
// }

// a00ZZVUVIJZ-0#e3aZZVUVIZJ-0#
//
// a -> on (action)
// 0 -> 0 second
// 0 -> 0 no timer id so we would have do '0' - val
// ZZVUVIJZ-0 -> code
// # separator
// e -> on relay 2 protocol
// 3 -> 3*3 = 9 seconds
// a -> 3 timer id
// ZZVUVIZJ-0 -> code
// # separator end

void Rf::init_store()
{
    // to fix
    // int storeItem = -1;
    // for(int pos = EEPROM_RF_STORE_START; pos < EEPROM_RF_STORE_SIZE; pos++) {
    //     int bit = EEPROM.read(pos);
    //     if (bit < 'a' || bit > 'l') {
    //         break; // unknown action
    //     } else {
    //         storeItem++;
    //         store[storeItem].action = bit;
    //         store[storeItem].timer = EEPROM.read(++pos) - '0'; // let's start at '0'
    //         store[storeItem].timer_id = EEPROM.read(++pos) - '0'; // 0 is no id, let's start at '0'
    //         for(int codePos = 0; pos < EEPROM_RF_STORE_SIZE && codePos < RF_RESULT_SIZE+2; codePos++) {
    //             bit = EEPROM.read(++pos);
    //             if (bit == '#') {
    //                 break; // separator
    //             }
    //             store[storeItem].code[codePos] = bit;
    //         }
    //     }
    // }
}

void Rf::trigger_action_timer(Action * object, int action, int timer, int timer_id) {
    if (timer > 0) {
        add_timer(object, action, timer * timer, timer_id);
    } else {
        (* object)(action);
    }
}

Action * Rf::get_relay(int action)
{
    int relay = (action - 'a') * 0.25;
    Action * object;
    if (relay == 0) { // abc
        object = &Relay1;
    }
#ifdef PIN_RELAY_2 // def
    else if (relay == 1) {
        object = &Relay1;
    }
#endif
#ifdef PIN_RELAY_3 // ghi
    else if (relay == 2) {
        object = &Relay1;
    }
#endif
#ifdef PIN_RELAY_4 // jkl
    else if (relay == 3) {
        object = &Relay1;
    }
#endif
    else {
        object = &Relay1;
    }
    return object;
}

void Rf::trigger_action(int action, int timer, int timer_id)
{
    int state = (action - 'a') % 3;
    Action * object = get_relay(action);
    if (state == 0) {
        trigger_action_timer(object, ACTION_RELAY_ON, timer, timer_id);
    } else if (state == 1) {
        trigger_action_timer(object, ACTION_RELAY_OFF, timer, timer_id);
    } else if (state == 2) {
        trigger_action_timer(object, ACTION_RELAY_TOGGLE, timer, timer_id);
    }
}

void Rf::trigger(char * code)
{
    for(int item = 0; item < RF_STORE_SIZE; item++) {
        if (strcmp(store[item].code, code) == 0) {
            trigger_action(store[item].action, store[item].timer, store[item].timer_id);
            // break; // dont stop to get multiple event
        }
    }
}

void Rf::test()
{
    for(int yo=0;yo<RF_STORE_SIZE; yo++) {
        printf("trigger: %c t: %c tid: %c code: %s\n",
            store[yo].action,
            store[yo].timer + '0', // let s make it more readable
            store[yo].timer_id + '0',
            store[yo].code);
    }
}
