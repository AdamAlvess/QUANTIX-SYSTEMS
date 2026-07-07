#ifndef HISTO_LOGS_H
#define HISTO_LOGS_H

#include <Arduino.h>

// Structure pour stocker chaque événement d'alarme (EXF-16)
struct LogStructure {
    uint32_t timestamp;
    char codeAlarme[20];
    float valeurDeclenchement;
};

class HistoLogs {
private:
    static const int MAX_LOGS = 50; // Capacité de la mémoire circulaire (EXF-18)
    static LogStructure _logs[MAX_LOGS];
    static int _indexActuel;

public:
    static void initialiser();
    static void ajouterLog(const char* code, float valeur);
};

#endif // HISTO_LOGS_H