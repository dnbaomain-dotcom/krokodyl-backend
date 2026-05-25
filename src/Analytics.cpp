#include "Analytics.h"
#include <Arduino.h>
#include "LSM6DS3.h"
#include "BleGatt.h"
#include "PowerManager.h" // Tohle tu chybělo pro uspání!
#include <math.h>

extern LSM6DS3 myIMU; 

// --- STATE PROMĚNNÉ ---
static float top3Kicks[3] = {0.0, 0.0, 0.0}; 
static bool isKicking = false;
static float currentPeakG = 0.0;
static uint32_t kickStartTime = 0;
static uint32_t cooldownEndTime = 0;

// Proměnné pro spánek (přesunuto z .h)
static unsigned long lastActivityTime = 0;
const unsigned long SLEEP_TIMEOUT = 300000; // 5 minut

const int KICK_WINDOW_MS = 300; 
const int COOLDOWN_MS = 1000; 

// Výpočet Krokodýl Score
static int calculateFifaScore(float gForce) {
    float safeG = gForce;
    if (safeG > 24.0) safeG = 24.0;
    
    int score = pow((safeG / 24.0), 0.7) * 99;
    if (score > 99) score = 99;
    return score;
}

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
}

void Analytics::resetLeaderboard() {
    top3Kicks[0] = 0.0;
    top3Kicks[1] = 0.0;
    top3Kicks[2] = 0.0;
}

void Analytics::processFrame() {
    unsigned long currentTime = millis();

    // 1. NEJDŘÍV PŘEČTEME DATA ZE SENZORU
    float x = myIMU.readFloatAccelX();
    float y = myIMU.readFloatAccelY();
    float z = myIMU.readFloatAccelZ();
    
    float totalForce = sqrt((x*x) + (y*y) + (z*z));
    float currentG = fabs(totalForce - 1.0);

    // 2. HLÍDAČ POHYBU (Spánek)
    if (currentG > 1.2) {
        lastActivityTime = currentTime;
    }

    if (currentTime - lastActivityTime > SLEEP_TIMEOUT) {
        PowerManager::goToDeepSleep();
    }
    
    // 3. ANALYTIKA STŘELY (Pokud nejsme v cooldownu)
    if (millis() < cooldownEndTime) return;

    // Začátek okna
    if (currentG > 5.0 && !isKicking) {
        isKicking = true;
        kickStartTime = millis();
        currentPeakG = currentG;
    }

    // Jsme v okně
    if (isKicking) {
        if (currentG > currentPeakG) currentPeakG = currentG;

        // Konec okna
        if (millis() - kickStartTime > KICK_WINDOW_MS) {
            updateLeaderboard(currentPeakG);
            
            int finalScore = calculateFifaScore(currentPeakG);
            BleGatt::sendShotData((uint8_t)finalScore);

            isKicking = false;
            currentPeakG = 0.0;
            cooldownEndTime = millis() + COOLDOWN_MS;
        }
    }
}