#include <Arduino.h>
#include <Wire.h>
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/temperature/TMP126/com_spi.h"
#include "Middle_level/temperature/INA237/I2C_Init.h"


ComSpiTMP126 capteurTMP;

#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    maLedverte.begin();
    maLedrouge.begin();
    maLedverte.allumer();
    delay(500);
    maLedverte.eteindre();
    maLedrouge.allumer();
    delay(500);
    maLedrouge.eteindre();
    
    Serial.println("\n═══════════════════════════════════════════");
    Serial.println("   DEMARRAGE DU SYSTEME : AUTO-TEST   ");
    Serial.println("═══════════════════════════════════════════");
    delay(500); 

    bool systemOk = true;

    // ─── ÉTAPE 1 : INITIALISATION & CHECK INA237 (I2C) ───
    Serial.println("[CHECK] Initialisation du bus I2C & INA237...");
    // On passe les pins de ton choix et la fréquence par défaut (400kHz)
    if (!I2C_Init(PIN_I2C_SDA, PIN_I2C_SCL)) {
        Serial.println("[ERREUR CRITIQUE] Echec de la validation de l'INA237 sur le bus I2C.");
        systemOk = false;
    } else {
        Serial.println("[ OK ] INA237 initialise et reinitialise avec succes.");
    }

    Serial.println("-------------------------------------------");

    // ─── ÉTAPE 2 : INITIALISATION & CHECK TMP126 (SPI) ───
    Serial.println("[CHECK] Initialisation du bus SPI & TMP126...");
    TMP126_Erreur errTMP = capteurTMP.begin();
    
    if (errTMP != TMP126_OK) {
        Serial.printf("[ERREUR CRITIQUE] Echec du TMP126. Code Erreur SPI : %d\n", errTMP);
        if (errTMP == ERR_TMP126_DEVICE_ID) {
            Serial.println(" -> Motif : Le Device ID lu ne correspond pas a un TMP126 (Attendu: 0x126).");
        }
        systemOk = false;
    } else {
        Serial.println("[ OK ] TMP126 detecte et pret.");
    }

    // ─── VERDICT FINAL ───
    Serial.println("═══════════════════════════════════════════");
    if (systemOk) {
        Serial.println(" STATUS : [SUCCESS] Tous les composants sont opérationnels !");
        maLedverte.allumer();
    } else {
        Serial.println(" STATUS : [FAIL] Defaut composant detecte. Mode degrade.");
        maLedrouge.allumer();
    }
    Serial.println("═══════════════════════════════════════════\n");
}

void loop() {
    // Ton code principal ne s'exécutera que si nécessaire.
    // Tu peux aussi faire clignoter la LED en rouge ici si systemOk était false.
}