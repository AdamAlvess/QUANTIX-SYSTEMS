#include "buzzer.h"

Buzzer::Buzzer(uint8_t pin) {
    _pin = pin;
}

void Buzzer::begin() {
    pinMode(_pin, OUTPUT);
    eteindre();
}

void Buzzer::sonnerAvertissement() {
    // Une petite mélodie d'attention à deux tons
    tone(_pin, 1000, 150); // Fréquence 1000Hz pendant 150ms
    delay(200);
    tone(_pin, 1200, 150); // Fréquence 1200Hz pendant 150ms
    delay(200);
}

void Buzzer::sonnerCritique() {
    // Alarme stridente et rapide type "urgence"
    for (int i = 0; i < 3; i++) {
        tone(_pin, 1500, 80);  // Note très haute et très courte
        delay(100);
        tone(_pin, 800, 80);   // Note plus basse
        delay(100);
    }
}

void Buzzer::eteindre() {
    noTone(_pin);
}

// Définition de l'instance globale du Buzzer
Buzzer monBuzzer(13);