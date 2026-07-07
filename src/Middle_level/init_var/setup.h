#ifndef SETUP_H
#define SETUP_H

#include <Arduino.h>
// Inclusion des classes de traitement haut niveau
#include "Middle_level/temperature/TMP126/recup_donnee.h"  // Pour TMP126 et TMP126_Mesure
#include "Middle_level/temperature/sonde_ctn/recup_donnee.h" // Pour RecupDonnee (CTN)

// Déclarations extern pour le main.cpp
extern TMP126 capteurTMP; 
extern RecupDonnee sondePcb1;
extern RecupDonnee sondePcb2;

// Fonction globale d'initialisation
bool system_init();

#endif