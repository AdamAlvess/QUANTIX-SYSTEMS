#ifndef COM_ADC_H
#define COM_ADC_H

#include <Arduino.h>

class ComAdc {
private:
    uint8_t _pinLecture; // Ex: 26
    uint8_t _pinAlim;    // Ex: 25

public:
    // Le constructeur prend maintenant les deux pins
    ComAdc(uint8_t pinLecture, uint8_t pinAlim);
    void begin();
    int lireBrut();
};

#endif // COM_ADC_H