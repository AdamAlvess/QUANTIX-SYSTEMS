#include "led.h"

Led::Led(uint8_t pin) {
    _pin = pin;
}

void Led::begin() {
    pinMode(_pin, OUTPUT);
    eteindre(); // On s'assure qu'elle est éteinte au démarrage
}

void Led::allumer() {
    digitalWrite(_pin, HIGH);
}

void Led::eteindre() {
    digitalWrite(_pin, LOW);
}

// Définition des instances de LED avec leurs pins respectifs
Led maLedverte(14);
Led maLedrouge(15);