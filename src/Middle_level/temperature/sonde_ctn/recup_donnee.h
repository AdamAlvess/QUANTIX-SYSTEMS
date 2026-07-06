#ifndef RECUP_DONNEE_H
#define RECUP_DONNEE_H

#include "com_adc.h"
#include "algo_conv_math.h"
#include "erreur.h" // inclusion du nouveau fichier

class RecupDonnee {
private:
    ComAdc _adc;
    AlgoConvMath _math;
    ErreurSonde _diagnostic; // Instance du gestionnaire d'erreur

public:
    RecupDonnee(uint8_t pin);
    void begin();
    float obtenirTemperature();
    
    // Permet au main de savoir quel est le problème exact
    const char* obtenirStatutSonde() const { return _diagnostic.getCodeErreurString(); }
    bool estEnPanne() const { return _diagnostic.getEtat() != SONDE_OK; }
};

#endif // RECUP_DONNEE_H