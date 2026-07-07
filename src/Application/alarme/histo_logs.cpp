#include "histo_logs.h"

// Initialisation des variables statiques
LogStructure HistoLogs::_logs[MAX_LOGS];
int HistoLogs::_indexActuel = 0;

void HistoLogs::initialiser() {
    _indexActuel = 0;
}

void HistoLogs::ajouterLog(const char* code, float valeur) {
    _logs[_indexActuel].timestamp = millis(); // Remplace par un RTC ou temps système si dispo
    
    // Copie sécurisée de la chaîne
    strncpy(_logs[_indexActuel].codeAlarme, code, sizeof(_logs[_indexActuel].codeAlarme) - 1);
    _logs[_indexActuel].codeAlarme[sizeof(_logs[_indexActuel].codeAlarme) - 1] = '\0';
    
    _logs[_indexActuel].valeurDeclenchement = valeur;

    // Écrasement circulaire automatique (Politique de rotation EXF-18)
    _indexActuel = (_indexActuel + 1) % MAX_LOGS; 
}