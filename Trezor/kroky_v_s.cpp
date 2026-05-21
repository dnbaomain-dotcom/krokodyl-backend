#include <Arduino.h>
#include "LSM6DS3.h"
#include "Wire.h"
#include <math.h>

LSM6DS3 myIMU(I2C_MODE, 0x6A);

// --- 🛠️ VÝVOJÁŘSKÝ PŘEPÍNAČ ---
// Změň na 'false', když dáš čip na achilovku
#define POCKET_MODE true 

#if POCKET_MODE
  const float STEP_IMPACT_THRESHOLD = 1.25; // Nízký práh (kapsa tlumí nárazy)
  const float SWING_GYRO_THRESHOLD = 60.0;  // Stehno se tolik netočí
#else
  const float STEP_IMPACT_THRESHOLD = 2.5;  // Tvrdý dopad paty
  const float SWING_GYRO_THRESHOLD = 150.0; // Ostrý švih lýtka
#endif

const unsigned long COOLDOWN_MS = 300;    
const unsigned long SWING_TIMEOUT_MS = 500; // V kapse trvá zhoupnutí déle

int stepCount = 0;
unsigned long lastStepTime = 0;
unsigned long lastSwingTime = 0;
bool isSwinging = false; 

float filteredAccel = 1.0;
const float alpha = 0.3; 
unsigned long lastSampleTime = 0;
const unsigned int SAMPLE_RATE_MS = 10; 

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  if (myIMU.begin() != 0) {
    Serial.println("❌ Chyba IMU!");
  } else {
    Serial.println("🐊👟 CHYTRY KROKOMETR AKTIVNI!");
    if(POCKET_MODE) Serial.println("⚠️ REZIM KAPSA AKTIVNI (Znizena citlivost)");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastSampleTime >= SAMPLE_RATE_MS) {
    lastSampleTime = currentMillis;

    float ax = myIMU.readFloatAccelX();
    float ay = myIMU.readFloatAccelY();
    float az = myIMU.readFloatAccelZ();
    float gx = myIMU.readFloatGyroX();
    float gy = myIMU.readFloatGyroY();
    float gz = myIMU.readFloatGyroZ();

    float rawAccel = sqrt((ax*ax) + (ay*ay) + (az*az));
    float gyroMag = sqrt((gx*gx) + (gy*gy) + (gz*gz));

    filteredAccel = (alpha * rawAccel) + ((1.0 - alpha) * filteredAccel);

    // 1. Detekce švihu (zhoupnutí nohy)
    if (gyroMag > SWING_GYRO_THRESHOLD) {
        isSwinging = true;
        lastSwingTime = currentMillis;
    }

    if (isSwinging && (currentMillis - lastSwingTime > SWING_TIMEOUT_MS)) {
        isSwinging = false;
    }

    // 2. Detekce dopadu
    if (filteredAccel > STEP_IMPACT_THRESHOLD) {
      if (isSwinging && (currentMillis - lastStepTime > COOLDOWN_MS)) {
        stepCount++;
        lastStepTime = currentMillis;
        isSwinging = false; 
        
        Serial.print("✅ KROK: ");
        Serial.print(stepCount);
        Serial.print(" | Dopad (G): ");
        Serial.println(filteredAccel, 2);
        
        // TODO pro L+P nohu: 
        // Zde se odešle čas (currentMillis) přes Bluetooth do druhého čipu.
        // Pokud přijde čas od druhého čipu a rozdíl je < 150ms -> Bude to SKOK (stepCount--)
      }
    }
  }
}