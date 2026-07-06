#include <Arduino.h>
#include <Wire.h>
#include "Middle_level/composant_interne/led.h"


// --- Adresses I2C (À adapter selon tes composants) ---
#define ADDR_INA   0x40 
#define ADDR_TMP   0x48    

// 5 18 23 19


// Fonction de diagnostic I2C
bool checkI2CDevice(uint8_t address, const char* name) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
        Serial.print("[ OK ] ");
        Serial.println(name);
        return true;
    } else {
        Serial.print("[ERREUR] ");
        Serial.print(name);
        Serial.print(" introuvable a l'adresse 0x");
        Serial.println(address, HEX);
        return false;
    }
}

// Fonction de diagnostic de la CTN
bool checkCTN(uint8_t pin) {
    int val = analogRead(pin);
    // Si la valeur est proche de 0 ou du max (ex: 4095 sur ESP32), 
    // c'est qu'il y a un défaut de câblage (court-circuit ou déconnexion)
    if (val < 10 || val > 4080) {
        Serial.print("[ERREUR] Sonde CTN : Valeur hors limite (");
        Serial.print(val);
        Serial.println("). Verifier le cablage ou la resistance de pull-up.");
        return false;
    }
    Serial.println("[ OK ] Sonde CTN");
    return true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10); // Attend l'ouverture du Serial
    
    Wire.begin(); // Initialisation du bus I2C
    maLedverte.begin();
    maLedrouge.begin();
    
    // On allume en bleu pendant le test (optionnel mais visuel !)
    maLedverte.allumer();
    delay(200);
    maLedverte.eteindre();
    maLedrouge.allumer();
    delay(200);
    maLedrouge.eteindre();

    Serial.println("\n--- DEMARRAGE DU SYSTEME : AUTO-TEST ---");
    delay(500); 

    bool systemOk = true;

    // 1. Check INA
    if (!checkI2CDevice(ADDR_INA, "Capteur INA")) {
        systemOk = false;
    }

    // 2. Check TMP
    if (!checkI2CDevice(ADDR_TMP, "Capteur TMP")) {
        systemOk = false;
    }

    // 3. Check Sonde CTN
    if (!checkCTN(34)) {
        systemOk = false;
    }

    // --- Verdict ---
    Serial.println("----------------------------------------");
    if (systemOk) {
        Serial.println("[SUCCESS] Tout fonctionne parfaitement !");
        maLedverte.allumer();
    } else {
        Serial.println("[FAIL] Echec du test composant. Mode degrade active.");
        maLedrouge.allumer();
    }
    Serial.println("----------------------------------------\n");
}

void loop() {
    // Ton code principal ici (qui ne tourne que si tout est OK, ou en mode dégradé)
}