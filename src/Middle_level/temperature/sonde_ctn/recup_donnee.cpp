#include "recup_donnee.h"

RecupDonnee::RecupDonnee(uint8_t pin) 
    : _adc(pin), _math(10000.0, 10000.0, 3950.0), _diagnostic(-10.0, 85.0) {}

void RecupDonnee::begin() {
    _adc.begin();
}

float RecupDonnee::obtenirTemperature() {
    int brut = _adc.lireBrut();
    
    // 1. Évaluation matérielle directe sur la valeur ADC brute
    if (_diagnostic.evaluerErreurMaterielle(brut) != SONDE_OK) {
        return -999.0; // On coupe court, inutile de faire des maths sur une panne franche
    }
    
    // 2. Conversion mathématique
    float tempCalculee = _math.convertirAdcEnCelsius(brut);
    
    // 3. Évaluation logique de la température obtenue
    if (_diagnostic.evaluerErreurThermique(tempCalculee) != SONDE_OK) {
        return -999.0;
    }
    
    return tempCalculee; // Tout est vert !
}