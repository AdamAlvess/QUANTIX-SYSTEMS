#include <Arduino.h>
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/composant_interne/buzzer.h"
#include "Application/alarme/erreur_temp_int.h"
#include "Application/alarme/erreur_temp_ext.h"
#include "Application/alarme/erreur_com.h"
#include "Application/ihm/enregistrement.h"

// Variables d'état du système (modifiées automatiquement par le scénario de test)
float temp_int = 35.0; // °C
float temp_ext = 25.0; // °C
float courant = 2.0;   // A
bool com_error = false;

// Flags pour le suivi des états d'alarmes
bool alarme_int_active = false;
bool alarme_ext_active = false;
bool alarme_com_active = false;

// Timers pour la gestion du temps non-bloquante
uint32_t dernier_temps_mesure = 0;
uint32_t dernier_temps_scenario = 0;

// Énumération des étapes du scénario de test automatique
enum EtapeScenario {
  ETAPE_NORMAL,
  ETAPE_AVERTISSEMENT_COM,
  ETAPE_AVERTISSEMENT_TEMP_EXT,
  ETAPE_CRITIQUE_TEMP_INT,
  ETAPE_EXTRACTION_LOGS
};

EtapeScenario etapeActuelle = ETAPE_NORMAL;

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; } // Attente de la console série
  
  // Initialisation des composants physiques
  maLedverte.begin();
  maLedrouge.begin();
  monBuzzer.begin();
  
  // Initialisation du gestionnaire d'enregistrement (RAM/PSRAM)
  initEnregistrement();
  
  // Log de démarrage
  enregistrerSystemLog("DEMARRAGE_ESP32");
  Serial.println("\n=== DEBUT DU TEST AUTOMATIQUE DES ALARMES ET LOGS ===");
}

void loop() {
  uint32_t tempsActuel = millis();

  // ==========================================
  // 1. SEQUENCEUR AUTOMATIQUE DE TEST (Toutes les 15 secondes)
  // ==========================================
  if (tempsActuel - dernier_temps_scenario >= 5000) {
    dernier_temps_scenario = tempsActuel;

    switch (etapeActuelle) {
      case ETAPE_NORMAL:
        Serial.println("\n>>> [SCENARIO]: Passage en etape AVERTISSEMENT COMMUNICATION");
        com_error = true;
        etapeActuelle = ETAPE_AVERTISSEMENT_COM;
        break;

      case ETAPE_AVERTISSEMENT_COM:
        Serial.println("\n>>> [SCENARIO]: Passage en etape AVERTISSEMENT TEMP EXTERIEURE");
        com_error = false;
        temp_ext = 52.0; // > 45°C
        etapeActuelle = ETAPE_AVERTISSEMENT_TEMP_EXT;
        break;

      case ETAPE_AVERTISSEMENT_TEMP_EXT:
        Serial.println("\n>>> [SCENARIO]: Passage en etape CRITIQUE TEMP INTERIEURE");
        temp_ext = 25.0;
        temp_int = 82.0; // > 75°C
        etapeActuelle = ETAPE_CRITIQUE_TEMP_INT;
        break;

      case ETAPE_CRITIQUE_TEMP_INT:
        Serial.println("\n>>> [SCENARIO]: Passage en etape EXTRACTION ET REINITIALISATION");
        temp_int = 35.0;
        etapeActuelle = ETAPE_EXTRACTION_LOGS;
        break;

      case ETAPE_EXTRACTION_LOGS:
        Serial.println("\n>>> [SCENARIO]: Dump automatique des logs accumules :");
        Serial.println("--- EXTRACTION DES LOGS PSRAM ---");
        extraireLogsSystem();
        extraireLogsAlarmes();
        extraireLogsMesures();
        Serial.println("--- FIN D'EXTRACTION ---");
        
        enregistrerSystemLog("RESET_AUTO_SCENARIO");
        Serial.println("\n>>> [SCENARIO]: Retour a l'etat NORMAL");
        etapeActuelle = ETAPE_NORMAL;
        break;
    }
  }

  // ==========================================
  // 2. ENREGISTREMENT PERIODIQUE DES MESURES (Toutes les 5 secondes)
  // ==========================================
  if (tempsActuel - dernier_temps_mesure >= 2000) {
    dernier_temps_mesure = tempsActuel;
    enregistrerMesure(courant, temp_int, temp_ext);
    Serial.print("."); // Indicateur visuel discret d'enregistrement de mesure
  }

  // ==========================================
  // 3. LOGIQUE D'EVALUATION EXCLUSIVE DES ALARMES
  // ==========================================
  
  // -- Priorité 1 : Alarme critique --
  if (temp_int > 75.0 || courant > 15.0) {
    maLedverte.eteindre();
    
    if (!alarme_int_active) {
      alarme_int_active = true;
      if (alarme_ext_active) { declencherAlarmeTempExt(false); alarme_ext_active = false; }
      if (alarme_com_active) { declencherAlarmeCom(false); alarme_com_active = false; }
      
      float val_declenchement = (temp_int > 75.0) ? temp_int : courant;
      enregistrerAlarme("TEMP_INT_OU_COURANT_CRITIQUE", val_declenchement);
    }
    declencherAlarmeTempInt(true);
  }
  
  // -- Priorité 2 : Avertissement thermique extérieur --
  else if (temp_ext > 45.0) {
    maLedverte.eteindre();
    
    if (!alarme_ext_active) {
      alarme_ext_active = true;
      if (alarme_int_active) { declencherAlarmeTempInt(false); alarme_int_active = false; }
      if (alarme_com_active) { declencherAlarmeCom(false); alarme_com_active = false; }
      
      enregistrerAlarme("TEMP_EXT_AVERTISSEMENT", temp_ext);
    }
    declencherAlarmeTempExt(true);
  }
  
  // -- Priorité 3 : Avertissement de communication --
  else if (com_error) {
    maLedverte.eteindre();
    
    if (!alarme_com_active) {
      alarme_com_active = true;
      if (alarme_int_active) { declencherAlarmeTempInt(false); alarme_int_active = false; }
      if (alarme_ext_active) { declencherAlarmeTempExt(false); alarme_ext_active = false; }
      
      enregistrerAlarme("ERREUR_COM_AVERTISSEMENT", 1.0);
    }
    declencherAlarmeCom(true);
  }
  
  // -- État Normal --
  else {
    if (alarme_int_active) { declencherAlarmeTempInt(false); alarme_int_active = false; }
    if (alarme_ext_active) { declencherAlarmeTempExt(false); alarme_ext_active = false; }
    if (alarme_com_active) { declencherAlarmeCom(false); alarme_com_active = false; }
    
    maLedverte.allumer();
    maLedrouge.eteindre();
    monBuzzer.eteindre();
  }

  delay(500); // Cadence d'évaluation stable
}