#include <Arduino.h>
#include <bluefruit.h>
#include "LSM6DS3.h"

LSM6DS3 myIMU(I2C_MODE, 0x6A);
BLEService shotService = BLEService(0x180D); 
BLECharacteristic shotPowerChar = BLECharacteristic(0x2A37); 

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // Vypnout LED

  myIMU.begin();

  // Inicializace Bluetooth
  Bluefruit.begin();
  Bluefruit.setName("KROKODYL_V1");
  shotService.begin();
  
  shotPowerChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  shotPowerChar.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  shotPowerChar.setFixedLen(1); 
  shotPowerChar.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addService(shotService);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start(0);
}

void loop() {
  float x = myIMU.readFloatAccelX();
  float y = myIMU.readFloatAccelY();
  float z = myIMU.readFloatAccelZ();
  float g = abs(sqrt(x*x + y*y + z*z) - 1.0);

  if (g > 5.0) { // Detekce rány
    digitalWrite(LED_BUILTIN, LOW); // Problikne LED na čipu
    
    // Výpočet skóre 0-99
    int score = pow((g / 24.0), 0.7) * 99;
    if (score > 99) score = 99;

    // ODESLÁNÍ DO APPKY
    uint8_t data = (uint8_t)score;
    shotPowerChar.notify8(data); 

    Serial.print("BOMBA: ");
    Serial.println(score);

    delay(200);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(800); // Cooldown
  }
  delay(10);
}