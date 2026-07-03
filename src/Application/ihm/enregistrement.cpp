#include "enregistrement.h"
#include <string.h>

// Pointeurs vers les buffers circulaires
MesureLog* mesuresBuffer = nullptr;
AlarmeLog* alarmesBuffer = nullptr;
SystemLog* systemLogsBuffer = nullptr;

// Indices d'écriture et compteurs pour la politique de rotation
int indexMesures = 0;
int countMesures = 0;

int indexAlarmes = 0;
int countAlarmes = 0;

int indexSystem = 0;
int countSystem = 0;

void initEnregistrement() {
    // Si la PSRAM est supportée matériellement par l'ESP32, on l'alloue dedans, sinon RAM interne
    #if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM_SUPPORT)
    if (psramFound()) {
        mesuresBuffer = (MesureLog*)ps_malloc(sizeof(MesureLog) * MAX_MESURES);
        alarmesBuffer = (AlarmeLog*)ps_malloc(sizeof(AlarmeLog) * MAX_ALARMES);
        systemLogsBuffer = (SystemLog*)ps_malloc(sizeof(SystemLog) * MAX_SYSTEM_LOGS);
        Serial.println("MEMOIRE: Buffers alloues avec succes en PSRAM.");
    } else {
    #endif
        mesuresBuffer = (MesureLog*)malloc(sizeof(MesureLog) * MAX_MESURES);
        alarmesBuffer = (AlarmeLog*)malloc(sizeof(AlarmeLog) * MAX_ALARMES);
        systemLogsBuffer = (SystemLog*)malloc(sizeof(SystemLog) * MAX_SYSTEM_LOGS);
        Serial.println("MEMOIRE: Buffers alloues en RAM interne (PSRAM non disponible).");
    #if defined(BOARD_HAS_PSRAM) || defined(CONFIG_SPIRAM_SUPPORT)
    }
    #endif
}

void enregistrerMesure(float courant, float temp_int, float temp_ext) {
    if (!mesuresBuffer) return;
    
    // Remplissage de l'entrée à l'index d'écriture actuel
    mesuresBuffer[indexMesures].timestamp = millis() / 1000;
    mesuresBuffer[indexMesures].courant = courant;
    mesuresBuffer[indexMesures].temp_int = temp_int;
    mesuresBuffer[indexMesures].temp_ext = temp_ext;
    
    // Logique de rotation : incrémenter l'index circulaire et écraser le plus ancien
    indexMesures = (indexMesures + 1) % MAX_MESURES;
    if (countMesures < MAX_MESURES) {
        countMesures++;
    }
}

void enregistrerAlarme(const char* description, float valeur) {
    if (!alarmesBuffer) return;
    
    alarmesBuffer[indexAlarmes].timestamp = millis() / 1000;
    strncpy(alarmesBuffer[indexAlarmes].description, description, sizeof(alarmesBuffer[indexAlarmes].description) - 1);
    alarmesBuffer[indexAlarmes].description[sizeof(alarmesBuffer[indexAlarmes].description) - 1] = '\0';
    alarmesBuffer[indexAlarmes].valeur = valeur;
    
    // Logique de rotation
    indexAlarmes = (indexAlarmes + 1) % MAX_ALARMES;
    if (countAlarmes < MAX_ALARMES) {
        countAlarmes++;
    }
}

void enregistrerSystemLog(const char* type_evt) {
    if (!systemLogsBuffer) return;
    
    systemLogsBuffer[indexSystem].timestamp = millis() / 1000;
    strncpy(systemLogsBuffer[indexSystem].type_evt, type_evt, sizeof(systemLogsBuffer[indexSystem].type_evt) - 1);
    systemLogsBuffer[indexSystem].type_evt[sizeof(systemLogsBuffer[indexSystem].type_evt) - 1] = '\0';
    
    // Logique de rotation
    indexSystem = (indexSystem + 1) % MAX_SYSTEM_LOGS;
    if (countSystem < MAX_SYSTEM_LOGS) {
        countSystem++;
    }
}

// Extraction des données (du plus ancien au plus récent)
void extraireLogsMesures() {
    if (!mesuresBuffer || countMesures == 0) {
        Serial.println("LOGS_MESURES_START:0");
        Serial.println("LOGS_MESURES_END");
        return;
    }
    
    Serial.print("LOGS_MESURES_START:");
    Serial.println(countMesures);
    
    // Calcul de l'index du plus ancien élément
    int startIdx = 0;
    if (countMesures == MAX_MESURES) {
        startIdx = indexMesures;
    }
    
    for (int i = 0; i < countMesures; i++) {
        int idx = (startIdx + i) % MAX_MESURES;
        Serial.print("MESURE:");
        Serial.print(mesuresBuffer[idx].timestamp);
        Serial.print(",");
        Serial.print(mesuresBuffer[idx].courant);
        Serial.print(",");
        Serial.print(mesuresBuffer[idx].temp_int);
        Serial.print(",");
        Serial.println(mesuresBuffer[idx].temp_ext);
    }
    Serial.println("LOGS_MESURES_END");
}

void extraireLogsAlarmes() {
    if (!alarmesBuffer || countAlarmes == 0) {
        Serial.println("LOGS_ALARMES_START:0");
        Serial.println("LOGS_ALARMES_END");
        return;
    }
    
    Serial.print("LOGS_ALARMES_START:");
    Serial.println(countAlarmes);
    
    int startIdx = 0;
    if (countAlarmes == MAX_ALARMES) {
        startIdx = indexAlarmes;
    }
    
    for (int i = 0; i < countAlarmes; i++) {
        int idx = (startIdx + i) % MAX_ALARMES;
        Serial.print("ALARME:");
        Serial.print(alarmesBuffer[idx].timestamp);
        Serial.print(",");
        Serial.print(alarmesBuffer[idx].description);
        Serial.print(",");
        Serial.println(alarmesBuffer[idx].valeur);
    }
    Serial.println("LOGS_ALARMES_END");
}

void extraireLogsSystem() {
    if (!systemLogsBuffer || countSystem == 0) {
        Serial.println("LOGS_SYSTEM_START:0");
        Serial.println("LOGS_SYSTEM_END");
        return;
    }
    
    Serial.print("LOGS_SYSTEM_START:");
    Serial.println(countSystem);
    
    int startIdx = 0;
    if (countSystem == MAX_SYSTEM_LOGS) {
        startIdx = indexSystem;
    }
    
    for (int i = 0; i < countSystem; i++) {
        int idx = (startIdx + i) % MAX_SYSTEM_LOGS;
        Serial.print("SYSTEM:");
        Serial.print(systemLogsBuffer[idx].timestamp);
        Serial.print(",");
        Serial.println(systemLogsBuffer[idx].type_evt);
    }
    Serial.println("LOGS_SYSTEM_END");
}