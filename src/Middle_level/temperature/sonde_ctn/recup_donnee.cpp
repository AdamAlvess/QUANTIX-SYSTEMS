#include "recup_donnee.h"

RecupDonnee::RecupDonnee(uint8_t pin) : _adc(pin), _math(10000.0, 10000.0, 3950.0) {}

void RecupDonnee::begin() {
    _adc.begin();
}

float RecupDonnee::obtenirTemperature() {
    int brut = _adc.lireBrut();
    return _math.convertirAdcEnCelsius(brut);
}