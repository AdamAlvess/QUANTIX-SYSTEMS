#ifndef RECUP_DONNEE_H
#define RECUP_DONNEE_H

#include "com_adc.h"
#include "algo_conv_math.h"

class RecupDonnee {
private:
    ComAdc _adc;
    AlgoConvMath _math;

public:
    // On passe le pin au constructeur
    RecupDonnee(uint8_t pin);
    
    void begin();
    float obtenirTemperature();
};

#endif // RECUP_DONNEE_H