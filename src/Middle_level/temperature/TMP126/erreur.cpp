#include "erreur.h"

const char* TMP126_ErreurVersTexte(TMP126_Erreur err) {
    switch (err) {
        case TMP126_OK:                 return "OK";
        case ERR_TMP126_DEVICE_ID:      return "ID capteur invalide (TMP126 non detecte)";
        case ERR_TMP126_SPI_TIMEOUT:    return "Timeout SPI";
        case ERR_TMP126_CRC:            return "Erreur CRC (ecriture rejetee par le capteur)";
        case ERR_TMP126_PARAMETRE:      return "Parametre d'appel invalide";
        case ERR_TMP126_NON_INITIALISE: return "Driver TMP126 non initialise (appeler init() d'abord)";
        default:                        return "Erreur TMP126 inconnue";
    }
}