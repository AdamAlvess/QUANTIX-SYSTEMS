#ifndef MODE_MAINTENANCE_H
#define MODE_MAINTENANCE_H

#include <Arduino.h>
#include "Middle_level/composant_interne/led.h" // Inclusion de ton gestionnaire de LED existant

class ModeMaintenance {
public:
    // Configure l'état du mode maintenance (manuel ou automatique)
    static void setActif(bool statut);

    // Indique si le mode maintenance est actif
    static bool isActif();

    // Gère le clignotement alterné (à appeler en boucle dans le loop)
    static void mettreAJourIhm();

private:
    static bool _estActif;
    static unsigned long _dernierChangementLed;
    static bool _phaseLedAlternée;
};

#endif // MODE_MAINTENANCE_H