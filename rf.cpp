
#include "rf.h"
#include "RCSwitch.h"
#include "config.h"
#include "log.h"
#include "web.h"
#include "EEPROM.h"
// #include "store.h"
#include "relay.h"
#include "timer.h"
#include "action.h"

void task_rf_t::task()
{
    logInfo("Start rf task\n");

    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableReceive(PIN_RF433_RECEIVER);

    while(true) {
        if (mySwitch.available()) {
            trigger(mySwitch.getReceivedCodeWord(), mySwitch.getReceivedProtocol());

            sprintf(cmd, ". rf/recv %lu %s %d %d\n", 
                mySwitch.getReceivedValue(),
                mySwitch.getReceivedCodeWord(),
                mySwitch.getReceivedBitlength(),
                mySwitch.getReceivedProtocol());

            web_send_all(cmd);

            mySwitch.resetAvailable();
        }
    }
}

void rf_save_store(char * data)
{
    EEPROM.save(EEPROM_RF_STORE_START, (uint8_t *)data, strlen(data));
}

// 0 ->   for timer (doesnt seem to work)
// '	' -> 9
// 12345678 protocol id
// 
// binary code:
// '0' -> 20
// '1' -> 21
// action:
// 'a' -> on
// 'b' -> off
// 'c' toggle
// 3? could be on for 5 min
void task_rf_t::init_store()
{
    int storeItem = -1;
    int codePos = 0;

    for(int pos = EEPROM_RF_STORE_START; pos < EEPROM_RF_STORE_SIZE; pos++) {
        int bit = EEPROM.read(pos);
        if (bit > 0 && bit < 9) {
            storeItem++;
            if (storeItem == RF_STORE_SIZE) {
                break;
            }
            store[storeItem].protocol = bit;
            printf("new protocol: %d\n", bit);
            codePos = 0;

            store[storeItem].timer = EEPROM.read(++pos) - 1; // protocol - 1 cause 0 does not work
            store[storeItem].timer_id = EEPROM.read(++pos);
        } else if (storeItem > -1) {
            if (bit == '0' || bit == '1')  { // 0 || 1
                if (codePos < RCSWITCH_MAX_CHANGES) {
                    store[storeItem].code[codePos] = bit;
                    codePos++;
                }
            } else if (bit >= 'a' && bit <= 'l') {
                store[storeItem].action = bit;
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

void task_rf_t::trigger_action_timer(Action * object, int action, int timer, int timer_id) {
    if (timer > 0) {
        add_timer(object, action, timer * timer, timer_id);
    } else {
        (* object)(action);
    }
}

Action * task_rf_t::get_relay(int action)
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

void task_rf_t::trigger_action(int action, int timer, int timer_id)
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

void task_rf_t::trigger(char * code, int protocol)
{
    for(int item = 0; item < RF_STORE_SIZE; item++) {
        if (store[item].protocol == protocol
          && strcmp(store[item].code, code) == 0) {
            trigger_action(store[item].action, store[item].timer, store[item].timer_id);
            // break; // dont stop to get multiple event
        }
    }
}

void task_rf_t::test()
{
    for(int yo=0;yo<RF_STORE_SIZE; yo++) {
        printf("uiiii: %d %d %s\n",
            store[yo].protocol,
            store[yo].action,
            store[yo].code);
    }
}
