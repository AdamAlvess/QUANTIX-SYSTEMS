#include "algo_conv_math.h"
#include <math.h>

AlgoConvMath::AlgoConvMath(float r_balance, float r0, float beta) 
    : _r_balance(r_balance), _r0(r0), _beta(beta), _t0(298.15), _adc_resolution(4095) {}

float AlgoConvMath::convertirAdcEnCelsius(int adc_brut) {
    if (adc_brut <= 0 || adc_brut >= _adc_resolution) {
        return -999.0; // Code d'erreur
    }
    
    // ─── LA CORRECTION ICI ───
    // Pour un pont diviseur avec CTN au VCC et R_balance au GND :
    // Vout = VCC * (R_balance / (R_ctn + R_balance))
    // Ce qui donne après inversion mathématique :
    float resistance_ctn = _r_balance * (((float)_adc_resolution / (float)adc_brut) - 1.0);
    
    // Si la formule précédente donnait l'inverse, c'est ce calcul qu'il faut appliquer :
    // resistance_ctn = _r_balance * ((float)adc_brut / ((float)_adc_resolution - (float)adc_brut));
    // Testons la version adaptée à la CTN en haut du pont :
    resistance_ctn = _r_balance * (((float)_adc_resolution / (float)adc_brut) - 1.0);
    
    // Équation Beta (Steinhart-Hart simplifiée)
    float kelvin = resistance_ctn / _r0;       
    kelvin = log(kelvin);                      
    kelvin /= _beta;                           
    kelvin += 1.0 / _t0;                       
    kelvin = 1.0 / kelvin;                     
    
    return kelvin - 273.15;
}