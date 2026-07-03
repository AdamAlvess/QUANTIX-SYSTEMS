#include <Arduino.h>
#include "Middle_level/composant_interne/led.h"
#include "Middle_level/composant_interne/buzzer.h"
#include "Application/alarme/erreur_temp_int.h"
#include "Application/alarme/erreur_temp_ext.h"
#include "Application/alarme/erreur_com.h"
#include "Application/ihm/enregistrement.h"

// Variables de simulation
float temp_int = 35.0; // °C
float temp_ext = 25.0; // °C
float courant = 2.0;   // A
bool com_error = false;

// Flags pour le suivi des états d'alarmes (eviter les doublons de logs et conflits)
bool alarme_int_active = false;
bool alarme_ext_active = false;
bool alarme_com_active = false;

// Temps du dernier enregistrement des mesures
uint32_t dernier_temps_mesure = 0;
bool tech_connecte = false;

void setup() {
  // Initialisation de la communication série pour le script de simulation
  Serial.begin(115200);
  while (!Serial) {
    ; // Attente de la connexion
  }
  
  // Initialisation des drivers physiques et de l'enregistrement
  maLedverte.begin();
  maLedrouge.begin();
  monBuzzer.begin();
  initEnregistrement();
  
  // Enregistrement du démarrage système
  enregistrerSystemLog("DEMARRAGE_ESP32");
  
  Serial.println("ESP32: Initialisation complete. Pret pour simulation.");
}

void loop() {
  // --- Analyseur de commandes série ---
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Journaliser la connexion technicien à la première commande reçue
    if (!tech_connecte) {
      tech_connecte = true;
      enregistrerSystemLog("CONNEXION_TECHNICIEN");
    }
    
    if (command.startsWith("TEMP_INT ")) {
      temp_int = command.substring(9).toFloat();
      Serial.print("SIMU: Temp interieure modifiee a "); Serial.print(temp_int); Serial.println(" C");
    } else if (command.startsWith("TEMP_EXT ")) {
      temp_ext = command.substring(9).toFloat();
      Serial.print("SIMU: Temp exterieure modifiee a "); Serial.print(temp_ext); Serial.println(" C");
    } else if (command.startsWith("CURRENT ")) {
      courant = command.substring(8).toFloat();
      Serial.print("SIMU: Courant modifie a "); Serial.print(courant); Serial.println(" A");
    } else if (command.startsWith("COM_ERROR ")) {
      com_error = (command.substring(10).toInt() == 1);
      Serial.print("SIMU: Erreur com modifiee a "); Serial.println(com_error ? "ACTIF" : "INACTIF");
    } else if (command == "RESET") {
      temp_int = 35.0;
      temp_ext = 25.0;
      courant = 2.0;
      com_error = false;
      Serial.println("SIMU: Reinitialisation de toutes les valeurs");
      enregistrerSystemLog("RESET_SIMULATION");
    } else if (command == "STATUS") {
      Serial.print("STATUS: TempInt="); Serial.print(temp_int);
      Serial.print(", TempExt="); Serial.print(temp_ext);
      Serial.print(", Current="); Serial.print(courant);
      Serial.print(", ComError="); Serial.println(com_error);
    } else if (command == "DUMP_LOGS") {
      Serial.println("--- EXTRACTION DES LOGS PSRAM ---");
      extraireLogsSystem();
      extraireLogsAlarmes();
      extraireLogsMesures();
      Serial.println("--- FIN D'EXTRACTION ---");
    }
  }

  // --- Enregistrement périodique des mesures (toutes les 5 secondes) ---
  if (millis() - dernier_temps_mesure >= 5000) {
    dernier_temps_mesure = millis();
    enregistrerMesure(courant, temp_int, temp_ext);
  }

  // --- Logique d'évaluation des alarmes locales séquentielle et exclusive ---
  
  // 1. Alarme critique : Température interne > 75°C ou Courant > 15A
  if (temp_int > 75.0 || courant > 15.0) {
    maLedverte.eteindre();
    
    if (!alarme_int_active) {
      alarme_int_active = true;
      // Éteindre les autres alarmes si elles étaient actives
      if (alarme_ext_active) { declencherAlarmeTempExt(false); alarme_ext_active = false; }
      if (alarme_com_active) { declencherAlarmeCom(false); alarme_com_active = false; }
      
      float val_declenchement = (temp_int > 75.0) ? temp_int : courant;
      enregistrerAlarme("TEMP_INT_OU_COURANT_CRITIQUE", val_declenchement);
    }
    
    declencherAlarmeTempInt(true);
  }
  
  // 2. Avertissement thermique extérieur : Température extérieure > 45°C
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
  
  // 3. Avertissement de communication
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
  
  // 4. Retour à l'état normal
  else {
    // Si une alarme était précédemment active, on l'éteint proprement
    if (alarme_int_active) { declencherAlarmeTempInt(false); alarme_int_active = false; }
    if (alarme_ext_active) { declencherAlarmeTempExt(false); alarme_ext_active = false; }
    if (alarme_com_active) { declencherAlarmeCom(false); alarme_com_active = false; }
    
    maLedverte.allumer();
    maLedrouge.eteindre();
    monBuzzer.eteindre();
  }

  delay(500); // Fréquence d'évaluation et de traitement
}
