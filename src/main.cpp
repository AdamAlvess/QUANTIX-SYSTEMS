#include <Arduino.h>
#include "Middle_level/temperature/TMP126/recup_donnee.h"
#include "Application/mode_fonctionnement/mode_maintenance.h"
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/protocol_com/ble/donnee_api.h"

TMP126 capteurTemp126;

void setup() {
    Serial.begin(115200);
    
    // Initialisation de tes composants matériels existants
    maLedverte.begin();
    maLedrouge.begin();
    
    DonneeApi::init("AGV_MONITOR_ESP32");
    capteurTemp126.init();
    capteurTemp126.configurerSeuils(-10.0f, 75.0f, 5.0f);

    // Exemple : Déclenchement manuel pour tester ton clignotement alterné
    ModeMaintenance::setActif(true); 
}

void loop() {
    // Exécution continue du clignotement des LEDs si la maintenance est active
    ModeMaintenance::mettreAJourIhm();

    // Lecture nominale des données du châssis
    float courant_mesure = 14.20;
    float ntc_1 = 26.4;
    float ntc_2 = 27.1;
    float temp_amb_chassis = 0.0f;

    TMP126_Mesure mesure;
    if (capteurTemp126.lireMesure(mesure) == TMP126_OK) {
        temp_amb_chassis = mesure.temperature_c;
        
        // Si alerte mais en mode maintenance, on n'active pas le buzzer d'alarme (Confort technicien)
        if (mesure.alerte_haute && !ModeMaintenance::isActif()) {
            // buzzer.allumer();
        }
    }

    // Transmission à ton application Python via BLE
    if (DonneeApi::isConnected()) {
        // Optionnel : tu passes ici ton nouvel état de maintenance à ton API BLE
        DonneeApi::updateMeasurements(courant_mesure, ntc_1, ntc_2, temp_amb_chassis);
    }

    delay(50); // Petit délai court pour garder le loop fluide et réactif
}