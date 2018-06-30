
#ifndef __BUTTON_H__
#define __BUTTON_H__

class Button {
    public:
        Button(void (*callback)(void));
        Button(void (*callback)(void), void (*callback_short)(void));
        void init();

    // private:
    //     void handleButton(unsigned char pin);
};

#endif
