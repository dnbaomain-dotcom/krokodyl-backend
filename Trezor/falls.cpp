#include <Arduino.h>
#include "LSM6DS3.h"
#include "Wire.h"

// Inicializace senzoru na standardní I2C adrese pro XIAO BLE Sense
LSM6DS3 myIMU(I2C_MODE, 0x6A); 

// --- KONSTANTY PRO KALIBRACI (budeš muset ladit na hřišti) ---
// Hranice přetížení (G) pro detekci tupého nárazu
const float IMPACT_THRESHOLD = 4.0; 
// Hranice pro osu Z (G), kdy považujeme nohu za ležící (otočení o cca 90°)
const float Z_AXIS_HORIZONTAL_THRESHOLD = 0.4; 
// Jak dlouho po nárazu čekáme, než zkontrolujeme polohu (v milisekundách)
const unsigned long FALL_CHECK_DELAY = 1000; 

// --- STAVOVÉ PROMĚNNÉ ---
int drsnak_skore = 0; // Počet pádů
bool impact_detected = false;
unsigned long impact_time = 0;

void setup() {
    Serial.begin(115200);
    // while (!Serial); // Odkomentuj, pokud chceš čekat na otevření Serial Monitoru

    // Start I2C a senzoru
    if (myIMU.begin() != 0) {
        Serial.println("Kámo, senzor LSM6DS3 neodpovídá! Zkontroluj zapojení.");
        while (1);
    }
    Serial.println("Senzor ready. Jdeme počítat Drsňák skóre! ⚽💥");
}

void loop() {
    // 1. Vyčtení dat z akcelerometru (v jednotkách G)
    float ax = myIMU.readFloatAccelX();
    float ay = myIMU.readFloatAccelY();
    float az = myIMU.readFloatAccelZ();

    // 2. Výpočet celkové síly (Magnitude) - vektorový součet 3D zrychlení
    float magnitude = sqrt((ax * ax) + (ay * ay) + (az * az));

    // 3. Detekce nárazu (Impact)
    if (magnitude > IMPACT_THRESHOLD && !impact_detected) {
        impact_detected = true;
        impact_time = millis();
        Serial.println("BUM! Detekován silný náraz. Čekám na potvrzení pozice...");
    }

    // 4. Kontrola polohy ležmo po uplynutí krátkého času
    if (impact_detected) {
        // Počkáme sekundu po nárazu, aby se tělo "zklidnilo" na zemi
        if (millis() - impact_time > FALL_CHECK_DELAY) {
            
            // Přečteme aktuální polohu
            float current_ay = myIMU.readFloatAccelY();
            
            // Pokud je čip na achilovce a hráč leží, osa Z (která dřív mířila nahoru/dolů)
            // se otočí o 90 stupňů a přestane na ni působit gravitace (blíží se k 0G)
            if (abs(current_ay) < Z_AXIS_HORIZONTAL_THRESHOLD) {
                drsnak_skore++;
                Serial.print("PÁD POTVRZEN! Osa Z natočena. Aktuální Drsňák skóre: ");
                Serial.println(drsnak_skore);
                // ZDE PAK PŘIDÁŠ ODESLÁNÍ DO BLE / uložení
            } else {
                Serial.println("Falešný poplach! Hráč to ustál jako frajer.");
            }

            // Reset stavu pro zachycení dalšího pádu
            impact_detected = false;
        }
    }

    // Vzorkujeme cca 50x za sekundu (stačí pro nárazy a polohu)
    delay(20); 
}