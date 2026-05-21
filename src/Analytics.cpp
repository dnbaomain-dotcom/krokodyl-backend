#include "Analytics.h"
#include <Arduino.h>
#include "LSM6DS3.h"
#include "BleGatt.h"
#include <math.h>

extern LSM6DS3 myIMU; // Říkáme, že IMU existuje (definujeme v main.cpp)

// Naše "state" proměnné z tvého kódu schované hezky uvnitř analytiky
static float top3Kicks[3] = {0.0, 0.0, 0.0}; 
static bool isKicking = false;
static float currentPeakG = 0.0;
static uint32_t kickStartTime = 0;
static uint32_t cooldownEndTime = 0; // Nový neblokující timer

const int KICK_WINDOW_MS = 300; 
const int COOLDOWN_MS = 1000; // 1 sekunda pauza po ráně

// Výpočet Krokodýl Score
static int calculateFifaScore(float gForce) {
    float safeG = gForce;
    if (safeG > 24.0) safeG = 24.0;
    
    // Zde používáme tvůj mocninný vzorec
    int score = pow((safeG / 24.0), 0.7) * 99;
    if (score > 99) score = 99;
    return score;
}

// Uložení Top 3 střel do paměti čipu
static void updateLeaderboard(float newScore) {
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
}

void Analytics::init() {
    myIMU.begin();
    // Tady případně nastavíme senzor na maximální citlivost
}

void Analytics::resetLeaderboard() {
    top3Kicks[0] = 0.0;
    top3Kicks[1] = 0.0;
    top3Kicks[2] = 0.0;
}

void Analytics::processFrame() {
    // Pokud jsme v cooldownu po ráně, nic neměříme
    if (millis() < cooldownEndTime) return;

    float x = myIMU.readFloatAccelX();
    float y = myIMU.readFloatAccelY();
    float z = myIMU.readFloatAccelZ();
    
    float totalForce = sqrt((x*x) + (y*y) + (z*z));
    // FIX: fabs místo abs!
    float currentG = fabs(totalForce - 1.0);

    // 1. Zjistili jsme ránu nad 5 G? Začínáme nahrávat
    if (currentG > 5.0 && !isKicking) {
        isKicking = true;
        kickStartTime = millis();
        currentPeakG = currentG;
    }

    // 2. JSME V OKNĚ: Sledujeme, jestli síla nestoupá
    if (isKicking) {
        if (currentG > currentPeakG) currentPeakG = currentG;

        // 3. OKNO KONČÍ (uplynulo 300 ms)
        if (millis() - kickStartTime > KICK_WINDOW_MS) {
            updateLeaderboard(currentPeakG);
            
            // Převedeme surová G na Krokodýl Score
            int finalScore = calculateFifaScore(currentPeakG);
            
            // Odeslání do frontendu přes Bluetooth šuplíček!
            BleGatt::sendShotData((uint8_t)finalScore);

            // Ukončíme švih a zapneme neblokující cooldown
            isKicking = false;
            currentPeakG = 0.0;
            cooldownEndTime = millis() + COOLDOWN_MS;
        }
    }
}