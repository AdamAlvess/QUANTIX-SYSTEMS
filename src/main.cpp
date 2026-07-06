#include "Middle_level/temperature/sonde_ctn/recup_donnee.h"
// #include "Middle_level/temperature/capteur_numerique/ambiance.h" // À prévoir pour EXF-13

// EXF-11 : Déclaration des deux sondes analogiques du PCB
RecupDonnee sondePcb1(26); // MES_NTC_1
RecupDonnee sondePcb2(25); // MES_NTC_2

void setup() {
  Serial.begin(115200);
  sondePcb1.begin();
  sondePcb2.begin();
  // initCapteurNumerique(); // À prévoir pour EXF-13
}

void loop() {
  // Lecture des deux sondes thermiques du PCB
  float temp_pcb_1 = sondePcb1.obtenirTemperature();
  float temp_pcb_2 = sondePcb2.obtenirTemperature();
  
  // EXF-13 : Remplacer la simulation par la vraie lecture du capteur numérique
  // temp_int = capteurNumerique.lireAmbiance(); 

  // Pour ta logique d'alarme, tu prendras la température du PCB la plus haute :
  float temp_pcb_max = max(temp_pcb_1, temp_pcb_2);
  
  Serial.print("PCB Sonde 1 : "); Serial.print(temp_pcb_1);
  Serial.print(" °C | PCB Sonde 2 : "); Serial.print(temp_pcb_2);
  Serial.print(" °C | Max : "); Serial.println(temp_pcb_max);

  delay(1000);
}