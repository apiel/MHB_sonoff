
#include "rf.h"
#include "RCSwitch.h"
#include "config.h"
#include "log.h"
#include "cmd.h"

void task_rf_t::task()
{
    logInfo("# Start rf task\n");

    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableReceive(PIN_RF433_RECEIVER);  // wemos mini d6

    while(true) {
        if (mySwitch.available()) {
            char cmd[1024];
            sprintf(cmd, ". rf-recv %lu %s %d %d\n", 
                mySwitch.getReceivedValue(),
                mySwitch.getReceivedCodeWord(),
                mySwitch.getReceivedBitlength(),
                mySwitch.getReceivedProtocol());

            cmd_send(cmd);

            mySwitch.resetAvailable();
        }  
    }      
}
