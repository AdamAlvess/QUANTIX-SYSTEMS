#ifndef RECUP_DONNEE_TMP126_H
#define RECUP_DONNEE_TMP126_H

#include "com_spi.h"
#include "erreur.h"

// Structure exposée au reste du firmware (Application/alarme, histo_logs,
// API BLE...). Une mesure = une lecture complète et cohérente du capteur.
struct TMP126_Mesure {
    float    temperature_c;   // Température ambiante mesurée, en °C
    bool     donnee_prete;    // Data_Ready_Flag : une nouvelle conversion était dispo
    bool     alerte_haute;    // THigh_Flag : seuil haut franchi depuis la dernière lecture
    bool     alerte_basse;    // TLow_Flag  : seuil bas franchi depuis la dernière lecture
    bool     alerte_pente;    // Slew_Flag  : montée rapide de température détectée
    bool     erreur_crc;      // CRC_Flag   : la dernière écriture CRC a été rejetée
    uint32_t horodatage_ms;   // millis() au moment de la mesure (pour histo_logs)
};

class TMP126 {
public:
    // Initialise le bus SPI, vérifie la présence du capteur et applique une
    // configuration par défaut (conversion continue, mode comparateur).
    TMP126_Erreur init();

    // Configure les seuils d'alarme matériels du capteur (utilisés pour
    // positionner les flags lus dans lireMesure()).
    // NB : EXF-14 (isolation thermique) est une contrainte de placement PCB,
    // pas un paramètre logiciel ; elle n'intervient pas ici.
    TMP126_Erreur configurerSeuils(float seuilBasC, float seuilHautC, float hysteresisC);

    // Lit une mesure complète (température + statut d'alarme).
    // Fait 2 lectures simples et volontairement PAS une lecture en rafale :
    // Alert_Status ne s'efface que sur lecture simple sans auto-increment
    // (cf. com_spi.h / datasheet §8.6.3). Une lecture en rafale laisserait
    // les flags actifs en permanence.
    TMP126_Erreur lireMesure(TMP126_Mesure &mesure);

private:
    ComSpiTMP126 _com;
    bool _initialise = false;

    static float brut14VersCelsius(uint16_t brut);
    static uint16_t celsiusVers14Bits(float temperatureC);
};

#endif // RECUP_DONNEE_TMP126_H