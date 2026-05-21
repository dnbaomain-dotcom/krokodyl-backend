#include "BleGatt.h"
#include <bluefruit.h>

BLEService krokodylService(0x180D); 

// Šuplík 1: Střely (Premium) - odesílá 1 byte
BLECharacteristic shotPowerChar(0x2A37);

// Šuplík 2: Zápasové statistiky (Základní) - odesílá 5 bytů naráz
BLECharacteristic matchStatsChar(0x2A38); 

// Šuplík 3: Kritické alarmy (Pády, Zranění) - odesílá 1 byte
BLECharacteristic alertChar(0x2A39);

void BleGatt::init() {
    Bluefruit.begin();
    Bluefruit.setName("KROKODYL_V1");
    Bluefruit.autoConnLed(false); // Vražda modrého blikání

    krokodylService.begin();

    // Nastavení Střel
    shotPowerChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
    shotPowerChar.setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS);
    shotPowerChar.setFixedLen(1);
    shotPowerChar.begin();

    // Nastavení Zápasových statistik (Kroky + Kalorie + Agilita)
    matchStatsChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
    matchStatsChar.setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS);
    matchStatsChar.setFixedLen(5); // 2 byty kroky + 2 byty kalorie + 1 byte agilita
    matchStatsChar.begin();

    // Nastavení Alarmů
    alertChar.setProperties(CHR_PROPS_NOTIFY);
    alertChar.setPermission(SECMODE_ENC_NO_MITM, SECMODE_NO_ACCESS);
    alertChar.setFixedLen(1);
    alertChar.begin();

    Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
    Bluefruit.Advertising.addService(krokodylService);
    Bluefruit.Advertising.addName();
    Bluefruit.Advertising.start(0); 
}

void BleGatt::sendShotData(uint8_t score) {
    shotPowerChar.write8(score);
    shotPowerChar.notify8(score);
}

void BleGatt::updateMatchStats(uint16_t steps, uint16_t calories, uint8_t agility) {
    uint8_t data[5];
    // Zabalení čísel do bytů (React appka si to pak zase složí)
    data[0] = steps & 0xFF;        
    data[1] = (steps >> 8) & 0xFF; 
    data[2] = calories & 0xFF;
    data[3] = (calories >> 8) & 0xFF;
    data[4] = agility;

    matchStatsChar.write(data, 5);
    matchStatsChar.notify(data, 5);
}

void BleGatt::sendAlert(uint8_t alertCode) {
    alertChar.write8(alertCode);
    alertChar.notify8(alertCode);
}