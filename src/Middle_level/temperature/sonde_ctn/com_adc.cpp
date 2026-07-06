#include "com_adc.h"

ComAdc::ComAdc(uint8_t pin) : _pin(pin) {}

void ComAdc::begin() {
    pinMode(_pin, INPUT);
}

int ComAdc::lireBrut() {
    return analogRead(_pin);
}