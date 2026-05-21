#include <Arduino.h>
#include <bluefruit.h>
#include "LSM6DS3.h"
#include <math.h>

LSM6DS3 myIMU(I2C_MODE, 0x6A);

BLEService        shotService = BLEService(0x180D); 
BLECharacteristic shotPowerChar = BLECharacteristic(0x2A37); 

void setupBLE() {
  Bluefruit.begin();
  Bluefruit.setName("KROKODYL_V1");

  shotService.begin();

  // ZMĚNA: Přidali jsme CHR_PROPS_READ. Tohle donutí iPhone ukázat všechna tlačítka!
  shotPowerChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  shotPowerChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  shotPowerChar.setFixedLen(1); 
  shotPowerChar.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(shotService);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start(0); 
}

void setup() {
  Serial.begin(9600);
  // ZMĚNA: Smazali jsme while(!Serial); Čip teď funguje i bez zapnutého monitoru na Macu!
  
  pinMode(LED_BUILTIN, OUTPUT); // Aktivujeme žárovku na desce
  digitalWrite(LED_BUILTIN, HIGH); // Vypneme ji (u XIAO desek HIGH většinou znamená vypnuto)

  myIMU.begin();
  setupBLE();
}

void loop() {
  float x = myIMU.readFloatAccelX();
  float y = myIMU.readFloatAccelY();
  float z = myIMU.readFloatAccelZ();
  float g = abs(sqrt(x*x + y*y + z*z) - 1.0);

  if (g > 5.0) { 
    // VIZUÁLNÍ KONTROLA: Zapneme LEDku, abychom věděli, že jsme trefili ránu!
    digitalWrite(LED_BUILTIN, LOW); 

    int score = pow((g / 24.0), 0.7) * 99;
    if (score > 99) score = 99;

    Serial.print("POSILAM DO BLUETOOTH: ");
    Serial.println(score);

    uint8_t data = (uint8_t)score;
    shotPowerChar.write8(data);  // Uložíme do šuplíčku (pro READ)
    shotPowerChar.notify8(data); // Zakřičíme do okolí (pro NOTIFY)

    delay(200); // Necháme chvíli svítit LEDku
    digitalWrite(LED_BUILTIN, HIGH); // Vypneme LEDku
    delay(800); // Zbytek cooldownu
  }
  delay(10);
}