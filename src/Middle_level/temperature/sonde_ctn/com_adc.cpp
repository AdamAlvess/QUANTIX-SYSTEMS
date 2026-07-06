#include "com_adc.h"

ComAdc::ComAdc(uint8_t pinLecture, uint8_t pinAlim) 
    : _pinLecture(pinLecture), _pinAlim(pinAlim) {}

void ComAdc::begin() {
    pinMode(_pinLecture, INPUT);
    pinMode(_pinAlim, OUTPUT);
    digitalWrite(_pinAlim, LOW); // Éteint par défaut pour économiser l'énergie
}

int ComAdc::lireBrut() {
    digitalWrite(_pinAlim, HIGH); // On alimente le pont diviseur juste pour la mesure
    delayMicroseconds(20);        // On laisse la tension se stabiliser
    int valeur = analogRead(_pinLecture);
    digitalWrite(_pinAlim, LOW);  // On rééteint
    return valeur;
}