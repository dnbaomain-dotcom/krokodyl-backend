#include <Arduino.h>
#include "LSM6DS3.h"
#include "Wire.h"
#include <math.h> // Potřebujeme pro funkci pow()

LSM6DS3 myIMU(I2C_MODE, 0x6A);

float top3Kicks[3] = {0.0, 0.0, 0.0}; 
bool isKicking = false;
float currentPeakG = 0.0;
unsigned long kickStartTime = 0;
const int KICK_WINDOW_MS = 300; 

void setup() {
  Serial.begin(9600);
  while (!Serial);
  myIMU.begin();
  Serial.println("=====================================");
  Serial.println("KROKODYLI RADAR v1.0 - HARDCORE MOD 🐊");
  Serial.println("=====================================");
}

// Funkce, která převede G na body s naší novou matematikou
int calculateFifaScore(float gForce) {
  // Zastropujeme hodnotu na 24G, aby nám nevzniklo skóre 105
  float safeG = gForce;
  if (safeG > 24.0) safeG = 24.0;
  
  // Náš nový mocninný vzorec
  int score = pow((safeG / 24.0), 0.7) * 99;
  return score;
}

// Funkce pro získání slovního hodnocení
String getRank(int score) {
  if (score < 40) return "Pripravka 👶";
  if (score < 75) return "Amater 🏃‍♂️";
  if (score < 90) return "Profik ⚽🔪";
  return "RAKETA (Roberto Carlos) 🚀🔥";
}

void updateLeaderboard(float newScore) {
  if (newScore <= top3Kicks[2]) return;

  if (newScore > top3Kicks[0]) {
    top3Kicks[2] = top3Kicks[1];
    top3Kicks[1] = top3Kicks[0];
    top3Kicks[0] = newScore;
  } else if (newScore > top3Kicks[1]) {
    top3Kicks[2] = top3Kicks[1];
    top3Kicks[1] = newScore;
  } else {
    top3Kicks[2] = newScore;
  }

  Serial.println("\n--- 🏆 TOP 3 RÁNY ZÁPASU 🏆 ---");
  for(int i=0; i<3; i++) {
    if (top3Kicks[i] > 0) {
      int score = calculateFifaScore(top3Kicks[i]);
      Serial.print(i+1);
      Serial.print(". misto: ");
      Serial.print(score);
      Serial.print(" OVR | ");
      Serial.print(top3Kicks[i]);
      Serial.print(" G | ");
      Serial.println(getRank(score));
    }
  }
  Serial.println("---------------------------------");
}

void loop() {
  float x = myIMU.readFloatAccelX();
  float y = myIMU.readFloatAccelY();
  float z = myIMU.readFloatAccelZ();
  float totalForce = sqrt((x*x) + (y*y) + (z*z));
  float currentG = abs(totalForce - 1.0);

  // START švihu (zvednuto na 5G)
  if (currentG > 5.0 && !isKicking) {
    isKicking = true;
    kickStartTime = millis();
    currentPeakG = currentG;
  }

  // NAHRÁVÁNÍ špičky
  if (isKicking) {
    if (currentG > currentPeakG) currentPeakG = currentG;

    // KONEC švihu (po 300 ms)
    if (millis() - kickStartTime > KICK_WINDOW_MS) {
      updateLeaderboard(currentPeakG);
      isKicking = false;
      currentPeakG = 0.0;
      delay(1000); 
    }
  }
  delay(10);
}