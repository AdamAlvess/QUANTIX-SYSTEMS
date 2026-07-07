#include <Arduino.h>
#include "Middle_level/init_var/setup.h"
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/temperature/INA237/INA237_Temp.h"

bool systemOk = false;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    // Appel unique de l'auto-test défini dans setup.cpp
    systemOk = system_init();
}

void loop() {
    if (!systemOk) {
        // Mode dégradé : Clignotement d'alerte rouge
        maLedrouge.allumer(); delay(200);
        maLedrouge.eteindre(); delay(200);
        return; 
    }

    Serial.println("--- NOUVELLE CAPTURE ---");

    // ─── 1. LECTURE INA237 ───
    float ina_voltage = INA237_ReadVoltageFromTemp(); 

    // ─── 2. LECTURE TMP126 ───
    TMP126_Mesure mesureTMP; 
    TMP126_Erreur err = capteurTMP.lireMesure(mesureTMP); 
    
    if (err == TMP126_OK) {
        Serial.printf("[TMP126] Température : %.2f °C\n", 
                      mesureTMP.temperature_c);
    } else {
        Serial.printf("[TMP126] Erreur de lecture : %d\n", err);
    }

    // ─── 3. LECTURE CTN ───
    float temp_ctn1 = sondePcb1.obtenirTemperature();
    float temp_ctn2 = sondePcb2.obtenirTemperature();
    float temp_ctn_max = max(temp_ctn1, temp_ctn2);

    Serial.printf("[CTN]    Sonde 1 : %.2f °C | Sonde 2 : %.2f °C | Max : %.2f °C\n", 
                  temp_ctn1, temp_ctn2, temp_ctn_max);

    Serial.println("═══════════════════════════════════════════");
    delay(2000); 
=======
#include "groove.h"

GrooveI2C monBusGrove;

void setup() {
    Serial.begin(115200);
    delay(1000); // Laisser le temps au moniteur série de s'ouvrir
    Serial.println("=== Test du Bus I2C Partagé (Grove / INA237) ===");

    // 1. Initialisation du bus
    GrooveErreur err = monBusGrove.begin();
    if (err != GROOVE_OK) {
        Serial.println("Échec critique de l'initialisation du bus I2C !");
        while (1); // Bloque ici si l'I2C ne démarre pas
    }
    Serial.println("Bus I2C initialisé avec succès sur GPIO21 (SDA) et GPIO22 (SCL).");

    // 2. Test anti-conflit (EXF-31)
    Serial.print("Vérification de l'adresse 0x40 (INA237) : ");
    if (monBusGrove.verifierAdresseDisponible(0x40) == ERR_GROOVE_ADRESSE_CONFLIT) {
        Serial.println("Correct (L'adresse est bien détectée comme réservée !)");
    } else {
        Serial.println("Erreur (Le mécanisme anti-conflit n'a pas bloqué l'adresse).");
    }
}

void loop() {
    Serial.println("\n--- Lancement d'un scan du bus I2C ---");

    uint8_t adressesTrouvees[10] = {0};
    uint8_t nbTrouve = 0;

    GrooveErreur err = monBusGrove.scannerBus(adressesTrouvees, 10, nbTrouve);

    if (err == GROOVE_OK) {
        Serial.printf("Scan terminé. Nombre de périphériques répondant : %d\n", nbTrouve);
        
        for (uint8_t i = 0; i < nbTrouve; i++) {
            Serial.printf(" -> Périphérique détecté à l'adresse : 0x%02X", adressesTrouvees[i]);
            
            // Petit clin d'œil à ton fichier de config
            if (adressesTrouvees[i] == 0x40) {
                Serial.print(" (INA237 - Mesure de courant)");
            } else {
                Serial.print(" (Module externe branché sur connecteur Grove)");
            }
            Serial.println();
        }
    } else {
        Serial.println("Erreur lors du scan du bus.");
    }

    delay(5000); // Attendre 5 secondes avant le prochain scan
}