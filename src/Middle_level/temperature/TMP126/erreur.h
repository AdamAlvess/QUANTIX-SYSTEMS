#ifndef ERREUR_TMP126_H
#define ERREUR_TMP126_H

#include <stdint.h>

// Codes d'erreur remontés par le driver TMP126 (com_spi + recup_donnee).
// A remonter ensuite vers Application/alarme/erreur_temp_ext pour
// l'enregistrement dans l'historique (histo_logs) et la gestion des alarmes.
enum TMP126_Erreur : uint8_t {
    TMP126_OK = 0,

    ERR_TMP126_DEVICE_ID,       // Device_ID lu != 0x126 -> capteur absent / mal câblé / SPI KO
    ERR_TMP126_SPI_TIMEOUT,     // Pas de réponse du bus SPI
    ERR_TMP126_CRC,             // CRC_Flag levé : écriture rejetée par le capteur
    ERR_TMP126_PARAMETRE,       // Paramètre d'appel invalide (ex: seuilHaut <= seuilBas)
    ERR_TMP126_NON_INITIALISE   // Appel d'une méthode avant init()
};

// Petit helper pour les logs (println debug, histo_logs, etc.)
const char* TMP126_ErreurVersTexte(TMP126_Erreur err);

#endif // ERREUR_TMP126_H