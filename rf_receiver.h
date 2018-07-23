
#ifndef __RF_RECEIVER_H__
#define __RF_RECEIVER_H__

#define RF_MAX_CHANGES 256
#define RF_RESULT_SIZE 4 // 256 / 4 = 64
#define RF_SPLIT RF_MAX_CHANGES / RF_RESULT_SIZE
#define RF_MAX_FALSE 100

struct RfMinMax {
    uint16_t min;
    uint16_t max;
};

struct RfProtocol {
    const char * uid;
    RfMinMax latch;
    RfMinMax latch2;
    RfMinMax zero;
    RfMinMax one;
};

// 78690 could be second latch but doesnt fit in 65535
static const RfProtocol protocoles[] = {
    {"remote", { 7700, 7900 }, { 0, 0 }, { 180, 320 }, { 670, 810 }}
};

class RfReceiver
{
    public:
        // RfReceiver(RfProtocol protocoles);
        void start(int pin);
        void onInterrupt();

    protected:
        RfMinMax _mainLatch;
        int _currentProtocole = -1;
        unsigned int _timingsPos;
        unsigned int _timings[RF_MAX_CHANGES];
        unsigned long _lastTime = 0;
        unsigned int _falseTimingCount = 0;
        char _result[RF_RESULT_SIZE];
        void _setMainLatch();
        void _attachInterrupt(int pin);
        void _initCurrentProtocole(unsigned int duration);
        void _logTiming(unsigned int duration);
        void _checkForResult(unsigned int duration);
        bool _isInRange(unsigned int duration, const RfMinMax * minMax);
        bool _isLatch(unsigned int duration);
        bool _isZero(unsigned int duration);
        bool _isOne(unsigned int duration);
};

extern RfReceiver rf;

#endif
