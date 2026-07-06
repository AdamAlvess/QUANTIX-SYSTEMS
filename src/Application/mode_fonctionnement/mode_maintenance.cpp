#include "mode_maintenance.h"

// Initialisation des variables statiques
bool ModeMaintenance::_estActif = false;
unsigned long ModeMaintenance::_dernierChangementLed = 0;
bool ModeMaintenance::_phaseLedAlternée = false;

bool ModeMaintenance::isActif() {
    return _estActif;
}

void ModeMaintenance::setActif(bool statut) {
    _estActif = statut;
    
    if (_estActif) {
        Serial.println("[MAINTENANCE] Activation : Mode Maintenance actif (Priorité diagnostic).");
    } else {
        Serial.println("[MAINTENANCE] Désactivation : Retour au mode nominal.");
        // Sécurité : On éteint les LEDs pour laisser le mode nominal reprendre la main
        maLedrouge.eteindre();
        maLedverte.eteindre();
    }
}

void ModeMaintenance::mettreAJourIhm() {
    // Si le mode maintenance n'est pas actif, on ne fait rien (on laisse le mode nominal gérer les LEDs)
    if (!_estActif) return;

    // Cadencement du clignotement à 500ms sans bloquer l'ESP32
    unsigned long tempsActuel = millis();
    if (tempsActuel - _dernierChangementLed >= 500) {
        _dernierChangementLed = tempsActuel;
        _phaseLedAlternée = !_phaseLedAlternée; // Inversion de la phase

        if (_phaseLedAlternée) {
            maLedrouge.allumer();
            maLedverte.eteindre();
        } else {
            maLedrouge.eteindre();
            maLedverte.allumer();
        }
    }
}