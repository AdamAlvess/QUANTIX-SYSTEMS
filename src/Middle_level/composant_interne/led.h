#ifndef LED_H
#define LED_H

#include <Arduino.h>

class Led {
private:
    uint8_t _pin;

public:
    // Constructeur : prend le numéro de pin en paramètre
    Led(uint8_t pin);
    
    // Initialise le pin en mode OUTPUT
    void begin();
    
    // Actions
    void allumer();
    void eteindre();
};

extern Led maLedverte;
extern Led maLedrouge;

#endif // LED_H