
#include "rf.h"
#include "RCSwitch.h"
#include "config.h"
#include "log.h"
#include "web.h"
#include "EEPROM.h"
// #include "store.h"
#include "relay.h"
#include "timer.h"

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

            store[storeItem].timer = EEPROM.read(++pos) - 1;
        } else if (storeItem > -1) {
            if (bit == '0' || bit == '1')  { // 0 || 1
                if (codePos < RCSWITCH_MAX_CHANGES) {
                    store[storeItem].code[codePos] = bit;
                    codePos++;
                }
            } else if (bit == 'a' || bit == 'b' || bit == 'c') {
                store[storeItem].action = bit;
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

void task_rf_t::trigger_action_timer(void (*callback)(void), int timer) {
    if (timer > 0) {
        add_timer(callback, timer * timer);
    } else {
        callback();
    }
}

void task_rf_t::trigger_action(int action, int timer)
{
    if (action == 'a') {
        trigger_action_timer([]()->void{ Relay1.relay_on(); }, timer);
    } else if (action == 'b') {
        trigger_action_timer([]()->void{ Relay1.relay_off(); }, timer);
    } else if (action == 'c') {
        trigger_action_timer([]()->void{ Relay1.relay_toggle(); }, timer);
    }
#ifdef PIN_RELAY_2
    else if (action == 'd') {
        trigger_action_timer([]()->void{ Relay2.relay_on(); }, timer);
    } else if (action == 'e') {
        trigger_action_timer([]()->void{ Relay2.relay_off(); }, timer);
    } else if (action == 'f') {
        trigger_action_timer([]()->void{ Relay2.relay_toggle(); }, timer);
    }
#endif
#ifdef PIN_RELAY_3
    else if (action == 'g') {
        trigger_action_timer([]()->void{ Relay3.relay_on(); }, timer);
    } else if (action == 'h') {
        trigger_action_timer([]()->void{ Relay3.relay_off(); }, timer);
    } else if (action == 'i') {
        trigger_action_timer([]()->void{ Relay3.relay_toggle(); }, timer);
    }
#endif
#ifdef PIN_RELAY_4
    else if (action == 'j') {
        trigger_action_timer([]()->void{ Relay4.relay_on(); }, timer);
    } else if (action == 'k') {
        trigger_action_timer([]()->void{ Relay4.relay_off(); }, timer);
    } else if (action == 'l') {
        trigger_action_timer([]()->void{ Relay4.relay_toggle(); }, timer);
    }
#endif
}

void task_rf_t::trigger(char * code, int protocol)
{
    for(int item = 0; item < RF_STORE_SIZE; item++) {
        if (store[item].protocol == protocol
          && strcmp(store[item].code, code) == 0) {
            trigger_action(store[item].action, store[item].timer);
            break;
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
