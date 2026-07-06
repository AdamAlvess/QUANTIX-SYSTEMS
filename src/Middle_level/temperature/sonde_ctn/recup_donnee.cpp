#include "recup_donnee.h"

// On lui passe le pin de lecture (26) et d'alim (25)
RecupDonnee::RecupDonnee(uint8_t pinLecture) 
    : _adc(pinLecture, 25), _math(10000.0, 10000.0, 3950.0), _diagnostic(-10.0, 85.0) {}

void RecupDonnee::begin() {
    _adc.begin();
}

float RecupDonnee::obtenirTemperature() {
    int brut = _adc.lireBrut();
    
    // 1. Évaluation matérielle directe sur la valeur ADC brute
    if (_diagnostic.evaluerErreurMaterielle(brut) != SONDE_OK) {
        return -998.0; // On coupe court, inutile de faire des maths sur une panne franche
    }
    
    // 2. Conversion mathématique
    float tempCalculee = _math.convertirAdcEnCelsius(brut);
    
    // 3. Évaluation logique de la température obtenue
    if (_diagnostic.evaluerErreurThermique(tempCalculee) != SONDE_OK) {
        return -997.0;
    }
    
    return tempCalculee; // Tout est vert !
}