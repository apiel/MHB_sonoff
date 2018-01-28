
#include "rf.h"
#include "RCSwitch.h"
#include "config.h"
#include "log.h"
#include "web.h"
#include "EEPROM.h"
#include "store.h"

void task_rf_t::task()
{
    logInfo("# Start rf task\n");

    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableReceive(PIN_RF433_RECEIVER);

    // char cmd[1024];
    while(true) {
        if (mySwitch.available()) {
            printf("...ToFix rf-recv %lu %s %d %d\n", 
                mySwitch.getReceivedValue(),
                mySwitch.getReceivedCodeWord(),
                mySwitch.getReceivedBitlength(),
                mySwitch.getReceivedProtocol());

            // seem to be a problem...
            // sprintf(cmd, ". rf-recv %lu %s %d %d\n", 
            //     mySwitch.getReceivedValue(),
            //     mySwitch.getReceivedCodeWord(),
            //     mySwitch.getReceivedBitlength(),
            //     mySwitch.getReceivedProtocol());

            // web_send_all(cmd);

            mySwitch.resetAvailable();
        }  
    }      
}

void rf_save_store(char * data)
{
    save_store(EEPROM_RF_STORE_START, data);
}

void task_rf_t::init_store()
{
    // 12345678 protocol id
    // binary code:
    // 0 -> 20
    // 1 -> 21
    // action:
    // 30 -> on
    // 31 -> off
    // 32 toggle
    // 3? could be on for 5 min
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
        } else if (storeItem > -1) {
            if (bit == 20 || bit == 21)  { // 0 || 1
                if (codePos < RCSWITCH_MAX_CHANGES) {
                    store[storeItem].code[codePos] = bit == 20 ? '0' : '1';
                    codePos++;
                }
            } else if (bit > 29 && bit < 33) {
                store[storeItem].action = bit;
            } else {
                break;
            }
        } else {
            break;
        }
    }

    printf("Store items: %d\n", storeItem);
    if (storeItem > -1) {
        for(int yo=0;yo<RF_STORE_SIZE; yo++) {
            printf("uiiii: %d %d %s\n",
                store[yo].protocol,
                store[yo].action,
                store[yo].code);
        }
    }
}
