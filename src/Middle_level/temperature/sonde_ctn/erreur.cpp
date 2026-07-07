#include "erreur.h"

ErreurSonde::ErreurSonde(float tempMin, float tempMax) 
    : _etatActuel(SONDE_OK), _tempMinAbsolue(tempMin), _tempMaxAbsolue(tempMax) {}

TypeErreurSonde ErreurSonde::evaluerErreurMaterielle(int adc_brut) {
    if (adc_brut <= 5) {
        _etatActuel = SONDE_CIRCUIT_OUVERT;
    } else if (adc_brut >= 4090) {
        _etatActuel = SONDE_COURT_CIRCUIT;
    } else {
        _etatActuel = SONDE_OK; // Provisoirement valide tant qu'on n'a pas vérifié les maths
    }
    return _etatActuel;
}

TypeErreurSonde ErreurSonde::evaluerErreurThermique(float temperature) {
    // Si une erreur matérielle a déjà été détectée en amont, on la conserve
    if (_etatActuel != SONDE_OK) return _etatActuel;

    // Vérification des barrières physiques
    if (temperature < _tempMinAbsolue || temperature > _tempMaxAbsolue) {
        _etatActuel = SONDE_HORS_LIMITES;
    } else {
        _etatActuel = SONDE_OK;
    }
    return _etatActuel;
}

const char* ErreurSonde::getCodeErreurString() const {
    switch (_etatActuel) {
        case SONDE_OK:             return "OK";
        case SONDE_CIRCUIT_OUVERT: return "ERR_CIRCUIT_OUVERT";
        case SONDE_COURT_CIRCUIT:  return "ERR_COURT_CIRCUIT";
        case SONDE_HORS_LIMITES:   return "ERR_TEMP_HORS_LIMITES";
        default:                   return "UNKNOWN_ERR";
    }
}