#include "recup_donnee.h"

// On force explicitement la lecture sur 26 et l'alimentation sur 25

RecupDonnee::RecupDonnee(uint8_t pinLecture) 
    : _adc(pinLecture, pinLecture), _math(10000.0, 10000.0, 3950.0), _diagnostic(-10.0, 85.0) {}

void RecupDonnee::begin() {
    _adc.begin();
}

float RecupDonnee::obtenirTemperature() {
    int brut = _adc.lireBrut();
    
    Serial.print("DEBUG ADC BRUT (Pin 26): "); Serial.println(brut); 
    
    if (_diagnostic.evaluerErreurMaterielle(brut) != SONDE_OK) {
        return -998.0; 
    }
    
    float tempCalculee = _math.convertirAdcEnCelsius(brut);
    
    if (_diagnostic.evaluerErreurThermique(tempCalculee) != SONDE_OK) {
        return -997.0;
    }
    
    return tempCalculee;
}