#ifndef ENREGISTREMENT_H
#define ENREGISTREMENT_H

#include <Arduino.h>

// Tailles des buffers circulaires
#define MAX_MESURES 50
#define MAX_ALARMES 20
#define MAX_SYSTEM_LOGS 20

// 1. Structure de log pour les mesures périodiques
struct MesureLog {
    uint32_t timestamp;  // En secondes depuis le démarrage
    float courant;
    float temp_int;
    float temp_ext;
};

// 2. Structure de log pour les alarmes déclenchées
struct AlarmeLog {
    uint32_t timestamp;
    char description[32];
    float valeur;
};

// 3. Structure de log pour les événements système
struct SystemLog {
    uint32_t timestamp;
    char type_evt[32];
};

// Initialisation de la mémoire et des buffers
void initEnregistrement();

// Enregistrement des données (Politique de rotation automatique intégrée)
void enregistrerMesure(float courant, float temp_int, float temp_ext);
void enregistrerAlarme(const char* description, float valeur);
void enregistrerSystemLog(const char* type_evt);

// Extraction des données via le port Série (formatage pour le technicien)
void extraireLogsMesures();
void extraireLogsAlarmes();
void extraireLogsSystem();

#endif // ENREGISTREMENT_H