#ifndef ALGO_CONV_MATH_H
#define ALGO_CONV_MATH_H

class AlgoConvMath {
private:
    float _r_balance;
    float _r0;
    float _beta;
    float _t0;
    int _adc_resolution;

public:
    AlgoConvMath(float r_balance = 10000.0, float r0 = 10000.0, float beta = 3950.0);
    float convertirAdcEnCelsius(int adc_brut);
};

#endif // ALGO_CONV_MATH_H