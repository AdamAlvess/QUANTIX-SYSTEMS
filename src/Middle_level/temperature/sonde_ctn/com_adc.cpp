#include "com_adc.h"

// Nouveau constructeur simplifié pour le test
ComAdc::ComAdc(uint8_t pinLecture, uint8_t pinAlim) 
    : _pinLecture(pinLecture), _pinAlim(pinAlim) {}

void ComAdc::begin() {
    // On configure le pin de lecture
    pinMode(_pinLecture, INPUT);
    analogSetPinAttenuation(_pinLecture, ADC_11db);
    
    // On ne touche PAS au pin 25 au cas où c'est la deuxième sonde !
}

int ComAdc::lireBrut() {
    // Lecture directe (le pont diviseur est supposé alimenté par la carte)
    return analogRead(_pinLecture);
}