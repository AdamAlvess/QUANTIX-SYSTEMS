#include <Arduino.h>
#include "Middle_level/init_var/setup.h"
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/temperature/INA237/INA237_Temp.h"
#include "Middle_level/protocol_com/ble/donnee_api.h"

bool systemOk = false;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    // Appel unique de l'auto-test défini dans setup.cpp
    systemOk = system_init();

    // Initialisation du BLE au démarrage de l'ESP32
    DonneeApi::init("AGV_MONITOR_ESP32");
}

void loop() {
    if (!systemOk) {
        // Mode dégradé : Clignotement d'alerte rouge
        maLedrouge.allumer(); delay(200);
        maLedrouge.eteindre(); delay(200);
        return; 
    }

        // ─── 3. LECTURE CTN ───
    float temp_ctn1 = sondePcb1.obtenirTemperature();
    float temp_ctn2 = sondePcb2.obtenirTemperature();
    float temp_ctn_max = max(temp_ctn1, temp_ctn2);


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



    Serial.printf("[CTN]    Sonde 1 : %.2f °C | Sonde 2 : %.2f °C | Max : %.2f °C\n", 
                  temp_ctn1, temp_ctn2, temp_ctn_max);

    Serial.println("═══════════════════════════════════════════");
    delay(2000); 
}

#include <Arduino.h>
// Inclusion précise en fonction de votre arborescence :



void loop() {
    // Exemples de valeurs (à remplacer plus tard par vos capteurs réels)
    float courant_mesure = 12.80;
    float ntc_1 = 26.4;
    float ntc_2 = 27.1;
    float temp_amb_chassis = 22.5;

    // Si l'interface Quantix en Python se connecte, on lui envoie les données
    if (DonneeApi::isConnected()) {
        DonneeApi::updateMeasurements(courant_mesure, ntc_1, ntc_2, temp_amb_chassis);
    }

    delay(1000); // Tâche exécutée toutes les secondes
}

// nom esp32 "AGV_MONITOR_ESP32"