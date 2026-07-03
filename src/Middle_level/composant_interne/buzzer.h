#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer {
private:
    uint8_t _pin;

public:
    // Constructeur
    Buzzer(uint8_t pin);
    
    // Initialisation
    void begin();
    
    // Mélodies et sons
    void sonnerAvertissement();
    void sonnerCritique();
    void eteindre();
};

extern Buzzer monBuzzer;

#endif // BUZZER_H