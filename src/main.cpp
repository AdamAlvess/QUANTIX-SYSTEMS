#include <Arduino.h>
// Inclusions des deux middlewares
#include "Middle_level/protocol_com/ble/donnee_api.h"
#include "Middle_level/temperature/TMP126/recup_donnee.h"

// Déclaration de l'instance du capteur TMP126
TMP126 capteurTemp126;

void setup() {
    Serial.begin(115200);
    
    // 1. Initialisation du BLE au démarrage de l'ESP32
    DonneeApi::init("AGV_MONITOR_ESP32");

    // 2. Initialisation du capteur TMP126
    TMP126_Erreur err = capteurTemp126.init();
    if (err != TMP126_OK) {
        Serial.printf("Erreur init TMP126 : %s\n", TMP126_ErreurVersTexte(err));
        // -> a faire remonter vers Application/alarme/erreur_temp_ext
    }

    // Configuration des seuils (CDC §3.6) : alarme si T PCB > 75°C, hystérésis 5°C
    capteurTemp126.configurerSeuils(-10.0f, 75.0f, 5.0f);
}

void loop() {
    // 1. Variables encore statiques (en attendant le binôme mesure de courant / CTN)
    float courant_mesure = 12.80;
    float ntc_1 = 26.4;
    float ntc_2 = 27.1;
    
    // 2. Variable dynamique initialisée par défaut (au cas où le capteur échouerait)
    float temp_amb_chassis = 0.0f;

    // 3. Lecture réelle du capteur TMP126
    TMP126_Mesure mesure;
    TMP126_Erreur err = capteurTemp126.lireMesure(mesure);

    if (err == TMP126_OK) {
        // On remplace la valeur fictive par la vraie température issue du capteur
        temp_amb_chassis = mesure.temperature_c;

        // Log de debug pour toi dans le serial
        Serial.printf("[TMP126] T = %.3f C | alerte=%d\n", mesure.temperature_c, mesure.alerte_haute);
        
        if (mesure.alerte_haute) {
            // -> déclencher LED rouge + buzzer (EXF-25) + log PSRAM + notif BLE
        }
    } else {
        Serial.printf("Erreur lecture TMP126 : %s\n", TMP126_ErreurVersTexte(err));
    }

    // 4. Si l'interface Quantix en Python se connecte, on lui envoie les données réelles
    if (DonneeApi::isConnected()) {
        DonneeApi::updateMeasurements(courant_mesure, ntc_1, ntc_2, temp_amb_chassis);
        Serial.println("[BLE] Données envoyées à l'API !");
    }

    delay(1000); // Tâche exécutée toutes les secondes
}