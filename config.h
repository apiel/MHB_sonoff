
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define RELAY_ON 1
#define RELAY_OFF 0

#define ACTION_RELAY_ON 1
#define ACTION_RELAY_OFF 2
#define ACTION_RELAY_TOGGLE 3


#define DEVICE_ID "MHB_"

#define RELAY_1_NAME "Test light"
#define RELAY_2_NAME "Wemos2 light"

#define SONOFF 1
#ifdef SONOFF
    #define PIN_BUTTON 0 // D3
    #define PIN_LED 13
    #define PIN_RELAY_1 12
#endif

// #define SONOFF4CH 1
#ifdef SONOFF4CH
    #define PIN_BUTTON 0
    #define PIN_LED 13
    #define PIN_RELAY_1 12
    #define PIN_RELAY_2 5
    #define PIN_RELAY_3 4
    #define PIN_RELAY_4 15
#endif

// #define WEMOS 1
#ifdef WEMOS
    #define PIN_BUTTON 0 // D3
    #define PIN_LED 2
    #define PIN_RELAY_1 5 // D1
    // #define PIN_RELAY_2 5
#endif

#define RF_STORE {{ACTION_RELAY_ON, PIN_RELAY_1, 0, 0, "Ee:;]\\DC-0"}}

// https://community.blynk.cc/uploads/default/original/2X/4/4f9e2245bf4f6698e10530b9060595c893bf49a2.png
// D0 > GPIO 16
// D1 > GPIO 5
// D2 > GPIO 4 // WEMOS MINI LED
// D3 > GPIO 0 // when emitter connected bug on boot
// D4 > GPIO 2 // when emitter connected bug on boot
// D5 > GPIO 14
// D6 > GPIO 12
// D7 > GPIO 13
// D8 > GPIO 15

// #define UPNP

#endif
