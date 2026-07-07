#ifndef ERREUR_SONDE_H
#define ERREUR_SONDE_H

#include <Arduino.h>

// Énumération des états possibles de la sonde
enum TypeErreurSonde {
    SONDE_OK = 0,
    SONDE_CIRCUIT_OUVERT,  // Fil débranché (ADC proche de 0)
    SONDE_COURT_CIRCUIT,   // Fils en contact direct (ADC proche du max)
    SONDE_HORS_LIMITES     // Température incohérente avec l'usage
};

class ErreurSonde {
private:
    TypeErreurSonde _etatActuel;
    float _tempMinAbsolue;
    float _tempMaxAbsolue;

public:
    ErreurSonde(float tempMin = -10.0, float tempMax = 85.0);
    
    // Analyse la valeur brute de l'ADC pour détecter les pannes matérielles
    TypeErreurSonde evaluerErreurMaterielle(int adc_brut);
    
    // Analyse la température calculée pour détecter les anomalies logiques
    TypeErreurSonde evaluerErreurThermique(float temperature);
    
    // Récupérer l'état ou un texte lisible pour tes logs
    TypeErreurSonde getEtat() const { return _etatActuel; }
    const char* getCodeErreurString() const;
};

#endif // ERREUR_SONDE_H