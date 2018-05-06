#ifndef __ACTION_H__
#define __ACTION_H__

class Action {
    public:
        Action() {};
        virtual void operator() (int key) = 0;
};

#endif
