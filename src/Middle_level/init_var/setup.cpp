#include "setup.h"
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/temperature/INA237/I2C_Init.h"

// Définition des broches I2C
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

// Instanciation réelle des objets globaux
TMP126 capteurTMP; 
RecupDonnee sondePcb1(26); 
RecupDonnee sondePcb2(25); 

bool system_init() {
    bool success = true;

    // Test visuel des LEDs au démarrage
    maLedverte.begin();
    maLedrouge.begin();
    maLedverte.allumer(); delay(250); maLedverte.eteindre();
    maLedrouge.allumer(); delay(250); maLedrouge.eteindre();
    
    Serial.println("\n═══════════════════════════════════════════");
    Serial.println("   DEMARRAGE DU SYSTEME : AUTO-TEST   ");
    Serial.println("═══════════════════════════════════════════");

    // ─── ÉTAPE 1 : INA237 (I2C) ───
    Serial.println("[CHECK] Initialisation du bus I2C & INA237...");
    if (!I2C_Init(PIN_I2C_SDA, PIN_I2C_SCL, 400000)) {
        Serial.println("[ERREUR CRITIQUE] Echec de l'INA237 (I2C).");
        success = false;
    } else {
        Serial.println("[ OK ] INA237 détecté et réinitialisé.");
    }

    Serial.println("-------------------------------------------");

    // ─── ÉTAPE 2 : TMP126 (SPI) ───
    Serial.println("[CHECK] Initialisation du bus SPI & TMP126...");
    TMP126_Erreur errTMP = capteurTMP.init(); 
    if (errTMP != TMP126_OK) {
        Serial.printf("[ERREUR CRITIQUE] Échec TMP126. Code : %d\n", errTMP);
        success = false;
    } else {
        Serial.println("[ OK ] TMP126 configuré (Mode continu, 1 Hz).");
    }

    Serial.println("-------------------------------------------");

    // ─── ÉTAPE 3 : SONDES CTN ───
    Serial.println("[CHECK] Initialisation des sondes CTN...");
    sondePcb1.begin();
    sondePcb2.begin();
    
    float test_ctn1 = sondePcb1.obtenirTemperature();
    float test_ctn2 = sondePcb2.obtenirTemperature();
    
    if (test_ctn1 < -40.0 || test_ctn1 > 150.0 || test_ctn2 < -40.0 || test_ctn2 > 150.0) {
        Serial.printf("[ERREUR] Valeurs CTN aberrantes. CTN1: %.2f°C | CTN2: %.2f°C\n", test_ctn1, test_ctn2);
        success = false;
    } else {
        Serial.println("[ OK ] Sondes CTN opérationnelles.");
    }

    // ─── VERDICT FINAL ───
    Serial.println("═══════════════════════════════════════════");
    if (success) {
        Serial.println(" STATUS : [SUCCESS] Système nominal !");
        maLedverte.allumer();
    } else {
        Serial.println(" STATUS : [FAIL] Mode dégradé enclenché.");
        maLedrouge.allumer();
    }
    Serial.println("═══════════════════════════════════════════\n");

    return success;
}