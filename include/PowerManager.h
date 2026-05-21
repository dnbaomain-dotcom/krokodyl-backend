#pragma once
#include <Arduino.h>

class PowerManager {
public:
    static void init();
    static bool isCharging();
    static void checkFactoryReset();
    static void goToDeepSleep();
};