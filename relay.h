
#ifndef __RELAY_H__
#define __RELAY_H__

#include "config.h"

class Relay {
    public:
        Relay(int pin);
        void relay_on();
        void relay_off();
        void relay_toggle();
        int relay_status();

    protected:
        int _pin;
        int _status = -1;
        int _lastUpdate = 0;
        bool _can_update();
};

#ifdef PIN_RELAY
extern Relay Relay1;
#endif

#ifdef PIN_RELAY_2
extern Relay Relay2;
#endif

#ifdef PIN_RELAY_3
extern Relay Relay3;
#endif

#ifdef PIN_RELAY_4
extern Relay Relay4;
#endif

#endif
