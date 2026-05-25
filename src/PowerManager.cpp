#include "PowerManager.h"
#include <Arduino.h>
#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

#define RESET_PIN D1 // Příklad pinu, kam napájíš mikrotlačítko

uint32_t buttonPressStartTime = 0;

void PowerManager::init() {
    pinMode(LED_RED, OUTPUT);   digitalWrite(LED_RED, HIGH);
    pinMode(LED_GREEN, OUTPUT); digitalWrite(LED_GREEN, HIGH);
    pinMode(LED_BLUE, OUTPUT);  digitalWrite(LED_BLUE, HIGH);
}

bool PowerManager::isCharging() {
    // Fyzický dotaz na procesor ARM Cortex-M4, jestli je v USB proud
    return (NRF_POWER->USBREGSTATUS & POWER_USBREGSTATUS_VBUSDETECT_Msk) != 0;
}

void PowerManager::checkFactoryReset() {
    if (digitalRead(RESET_PIN) == LOW) { 
        if (buttonPressStartTime == 0) buttonPressStartTime = millis();
        
        // Pokud drží 10 vteřin a JE na nabíječce
        if ((millis() - buttonPressStartTime) > 10000) {
            if (isCharging()) {
                InternalFS.begin();
                InternalFS.format();
                // Zde později přidáme smazání Top 3 střel z paměti
                NVIC_SystemReset();     // Tvrdý restart čipu
            }
        }
    } else {
        buttonPressStartTime = 0;
    }
}

void PowerManager::goToDeepSleep() {
    Serial.println("Krokodýl usíná. Zzz...");
    
    // Bezpečné vypnutí Bluetooth vysílání
    Bluefruit.Advertising.stop();
    
    // Tímto systémovým příkazem pošleme nRF52 čip do nejhlubšího spánku.
    // Procesor se natvrdo vypne, probudí ho až reset.
    sd_power_system_off(); 
}