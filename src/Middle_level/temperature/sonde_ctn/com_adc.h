#ifndef COM_ADC_H
#define COM_ADC_H

#include <Arduino.h>

class ComAdc {
private:
    uint8_t _pin;

public:
    ComAdc(uint8_t pin);
    void begin();
    int lireBrut();
};

#endif // COM_ADC_H