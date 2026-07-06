#include "algo_conv_math.h"
#include <math.h>

AlgoConvMath::AlgoConvMath(float r_balance, float r0, float beta) 
    : _r_balance(r_balance), _r0(r0), _beta(beta), _t0(298.15), _adc_resolution(4095) {}

float AlgoConvMath::convertirAdcEnCelsius(int adc_brut) {
    if (adc_brut <= 0 || adc_brut >= _adc_resolution) {
        return -999.0; // Code d'erreur
    }
    
    // Calcul de la résistance de la CTN (pont diviseur avec CTN au VCC)
    float resistance_ctn = _r_balance * ((float)_adc_resolution / (float)adc_brut - 1.0);
    
    // Équation Beta
    float kelvin = resistance_ctn / _r0;       
    kelvin = log(kelvin);                      
    kelvin /= _beta;                           
    kelvin += 1.0 / _t0;                       
    kelvin = 1.0 / kelvin;                     
    
    return kelvin - 273.15;
}