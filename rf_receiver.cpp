
#include <espressif/esp_common.h>
#include <esp8266.h>
#include <math.h>

#include "rf_receiver.h"

enum {
   numProto = sizeof(protocoles) / sizeof(protocoles[0])
};

RfReceiver rf = RfReceiver();

void handleInterrupt(unsigned char pin)
{
    rf.onInterrupt();
}

void RfReceiver::start(int pin) {
    _setMainLatch();
    _attachInterrupt(pin);
}

void RfReceiver::_attachInterrupt(int pin) {
    gpio_enable(pin, GPIO_INPUT);
    printf("attach interrupt %d\n", pin);
    gpio_set_interrupt(pin, GPIO_INTTYPE_EDGE_ANY, handleInterrupt);
}

void RfReceiver::_setMainLatch() { // we could easily write unit test there
    _mainLatch = { 65535, 0 };
    printf("Yo %d and %d\n", _mainLatch.min, _mainLatch.max);
    for(unsigned int i = 0; i < numProto; i++) {
        printf("RF: Proto [%d: %s] between %d and %d\n", i, protocoles[i].uid, protocoles[i].latch.min, protocoles[i].latch.max);
        if (protocoles[i].latch.min < _mainLatch.min) {
            _mainLatch.min = protocoles[i].latch.min;
        }
        if (protocoles[i].latch.max > _mainLatch.max) {
            _mainLatch.max = protocoles[i].latch.max;
        }
    }
    printf("RF: Main latch between %d and %d\n", _mainLatch.min, _mainLatch.max);
}

void RfReceiver::onInterrupt() {
    long time = sdk_system_get_time();
    unsigned int duration = time - _lastTime;
    if (duration > _mainLatch.min && duration < _mainLatch.max) {
        _checkForResult(duration);
        _initCurrentProtocole(duration);
        printf("%d proto %d\n", duration, _currentProtocole);
    }
    _logTiming(duration);
    _lastTime = time;
}

// for the moment we support only one protocole at once
// but we could have an array of current protocole with { id, timings, ...}
void RfReceiver::_initCurrentProtocole(unsigned int duration) { // we could easily write unit test there
    for(_currentProtocole = numProto - 1; _currentProtocole > -1 ;_currentProtocole--) {
        if (_isLatch(duration)) {
             break;
        }
    }
    _result[0] = '0';
    _result[1] = '0';
    _result[2] = '0';
    _result[3] = '0';
    _falseTimingCount = 0;
    _timingsPos = 0;
}

void RfReceiver::_logTiming(unsigned int duration) {
    if (_currentProtocole > -1 && _timingsPos < RF_MAX_CHANGES) {
        if (_isZero(duration)) {
            _timings[_timingsPos] = 0;
            _timingsPos++;
        } else if (_isOne(duration)) {
            _timings[_timingsPos] = 1;
            // _result[_timingsPos/RF_SPLIT] += pow(2, _timingsPos);
            _timingsPos++;
        } else {
            _falseTimingCount++;
            if (_falseTimingCount > RF_MAX_FALSE) {
                _currentProtocole = -1;
            }
        }
    }
}

bool RfReceiver::_isInRange(unsigned int duration, const RfMinMax * minMax) {
    return duration > minMax->min && duration < minMax->max;
}

bool RfReceiver::_isLatch(unsigned int duration) {
    return _isInRange(duration, &protocoles[_currentProtocole].latch);
}

bool RfReceiver::_isZero(unsigned int duration) {
    return _isInRange(duration, &protocoles[_currentProtocole].zero);
}

bool RfReceiver::_isOne(unsigned int duration) {
    return _isInRange(duration, &protocoles[_currentProtocole].one);
}

void RfReceiver::_checkForResult(unsigned int duration) {
    if (_currentProtocole > -1 && _timingsPos > 0 && (_timingsPos >= RF_MAX_CHANGES || _isLatch(duration))) {
        printf("Result on protocole [%d] %s:\n", _currentProtocole, protocoles[_currentProtocole].uid);
        for(unsigned int i = 0; i < RF_MAX_CHANGES && i < _timingsPos; i++) {
            printf("%d,", _timings[i]);
        }
        printf("\ndone (%d) [false: %d]\n", _timingsPos, _falseTimingCount);
        _currentProtocole = -1;
    }
}
