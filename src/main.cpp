#include <Arduino.h>
#include "LSM6DS3.h" // Přidáno: Musíme znát knihovnu senzoru
#include "PowerManager.h"
#include "BleGatt.h"
#include "Analytics.h"

// TADY JE TEN CHYBĚJÍCÍ SENZOR! 
// Vytvoříme ho globálně, aby ho přes 'extern' mohl vidět i Analytics.cpp
LSM6DS3 myIMU(I2C_MODE, 0x6A);

void setup() {
    // 1. Zhasneme diody a připravíme tlačítko
    PowerManager::init();

    // 2. Nahodíme bezpečný Bluetooth (GATT + Bonding)
    BleGatt::init();

    // 3. Spustíme IMU senzor přes naši analytiku
    Analytics::init();
}

void loop() {
    // 4. Hlídáme 10vteřinové podržení tlačítka pro Factory Reset
    PowerManager::checkFactoryReset();

    // 5. Zpracování surových dat a detekce Krokodýlí střely
    Analytics::processFrame();

    // 6. Uvolnění procesoru do lehkého spánku mezi cykly
    delay(2); 
}