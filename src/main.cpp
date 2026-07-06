#include "Middle_level/temperature/sonde_ctn/recup_donnee.h"

// Instance globale de ton capteur sur le pin 34
RecupDonnee capteurTempExt(34);

void setup() {
  Serial.begin(115200);
  capteurTempExt.begin();
  // ... le reste de ton setup
}

void loop() {
  // Dans ton bloc de lecture de capteur physique :
  float vraie_temp_ext = capteurTempExt.obtenirTemperature();
  
  // ... le reste de ton loop qui traite l'automate de test et les alarmes
}