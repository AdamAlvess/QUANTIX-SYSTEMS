#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

#include <Arduino.h>
// Inclusion précise en fonction de votre arborescence :
#include "Middle_level/protocol_com/ble/donnee_api.h"

void setup() {
    Serial.begin(115200);
    
    // Initialisation du BLE au démarrage de l'ESP32
    DonneeApi::init("AGV_MONITOR_ESP32");
}

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