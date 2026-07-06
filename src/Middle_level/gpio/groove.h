#ifndef GROOVE_H
#define GROOVE_H

#include <Arduino.h>
#include <Wire.h>

// ---------------------------------------------------------------------------
// Connecteurs Grove (schéma "Connecteurs", feuille 7, J202 / J203)
//
// IMPORTANT : les 2 connecteurs Grove partagent le MEME bus I2C matériel
// (GPIO21 = SDA, GPIO22 = SCL), qui est également utilisé par l'INA237
// (mesure de courant, feuille "Mesure et acquisition"). Ce n'est donc pas
// 2 bus distincts mais 1 seul bus avec potentiellement 3 périphériques.
// -> c'est exactement le sujet de l'EXF-31 (pas de conflit d'adresse I2C).
//
//   Grove 1 (J202) : réservé au futur capteur de vibration (EXF-28)
//   Grove 2 (J203) : réservé au futur capteur d'humidité   (EXF-29)
//
// Les capteurs ne sont pas montés sur la carte au départ : cette couche doit
// donc fonctionner même si rien n'est branché (détection de présence),
// sans bloquer le reste du firmware.
// ---------------------------------------------------------------------------
#define GROOVE_I2C_SDA        21
#define GROOVE_I2C_SCL        22
#define GROOVE_I2C_FREQ_HZ    100000UL   // 100 kHz : compatible avec la quasi
                                         // totalité des modules Grove I2C

// Connecteur Grove logique (juste un identifiant pour les logs / l'API,
// les 2 connecteurs étant sur le même bus électrique)
enum GrooveConnecteur : uint8_t {
    GROOVE_1_VIBRATION = 0, // J202
    GROOVE_2_HUMIDITE  = 1  // J203
};

enum GrooveErreur : uint8_t {
    GROOVE_OK = 0,
    ERR_GROOVE_NON_INITIALISE,
    ERR_GROOVE_ADRESSE_INVALIDE,   // adresse 7 bits hors plage (0x08-0x77)
    ERR_GROOVE_ADRESSE_CONFLIT,    // adresse déjà utilisée par un autre device du bus
    ERR_GROOVE_CAPTEUR_ABSENT,     // pas d'ACK à l'adresse demandée (NACK)
    ERR_GROOVE_TRANSFERT          // échec lecture/écriture I2C (Wire)
};

// ---------------------------------------------------------------------------
// Table des adresses I2C déjà réservées sur le bus partagé, à tenir à jour
// au fur et à mesure des ajouts (exigée par EXF-31, à présenter au Jalon 1).
//
// TODO (à confirmer avec le binôme "mesure et acquisition") : l'adresse
// exacte de l'INA237 dépend du câblage des pins A0/A1 (R502/R503 sur le
// schéma). Valeur ci-dessous = adresse par défaut si A0=A1=GND.
// ---------------------------------------------------------------------------
struct GrooveAdresseReservee {
    const char* nomComposant;
    uint8_t     adresse7bits;
};

static const GrooveAdresseReservee GROOVE_ADRESSES_BUS_PARTAGE[] = {
    { "INA237 (mesure courant, feuille 5)", 0x40 } // A voir : dépend de A0/A1
    // Ajouter ici l'adresse du capteur de vibration (Grove 1) une fois choisi
    // Ajouter ici l'adresse du capteur d'humidité (Grove 2) une fois choisi
};
#define GROOVE_NB_ADRESSES_RESERVEES (sizeof(GROOVE_ADRESSES_BUS_PARTAGE) / sizeof(GROOVE_ADRESSES_BUS_PARTAGE[0]))

class GrooveI2C {
public:
    // Initialise le bus I2C partagé (Wire) sur GPIO21/GPIO22.
    GrooveErreur begin();

    // Vérifie qu'une adresse candidate (pour un futur capteur Grove) ne
    // rentre pas en conflit avec un composant déjà présent sur le bus.
    // A appeler avant d'ajouter une entrée dans un nouveau driver de capteur.
    GrooveErreur verifierAdresseDisponible(uint8_t adresse7bits);

    // Sonde une adresse I2C précise pour savoir si un capteur y répond
    // (permet de détecter la présence effective d'un module Grove optionnel
    // sans planter le firmware si le connecteur est vide).
    bool capteurPresent(uint8_t adresse7bits);

    // Scanne tout le bus (0x08 à 0x77) et retourne les adresses qui répondent.
    // Utile en debug / diagnostic technicien (EXF-19, consultation via BLE).
    GrooveErreur scannerBus(uint8_t *adressesTrouvees, uint8_t tailleMax, uint8_t &nombreTrouve);

    // Helpers génériques lecture/écriture registre 8 bits, réutilisables par
    // n'importe quel driver de capteur Grove (vibration, humidité, ...).
    GrooveErreur lireRegistre8(uint8_t adresse7bits, uint8_t reg, uint8_t &valeur);
    GrooveErreur ecrireRegistre8(uint8_t adresse7bits, uint8_t reg, uint8_t valeur);
    GrooveErreur lireRegistre16(uint8_t adresse7bits, uint8_t reg, uint16_t &valeur, bool msbFirst = true);

private:
    bool _initialise = false;
};

#endif // GROOVE_H