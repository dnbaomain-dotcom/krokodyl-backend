#pragma once
#include <stdint.h>

class BleGatt {
public:
    static void init();
    static void sendShotData(uint8_t score);
    // Nové: Pošle kroky, kalorie a agilitu v jednom balíčku
    static void updateMatchStats(uint16_t steps, uint16_t calories, uint8_t agility);
    // Nové: Pošle okamžitý alarm (pád = 1, asymetrie dopadu = 2)
    static void sendAlert(uint8_t alertCode);
};