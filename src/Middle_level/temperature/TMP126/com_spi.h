#ifndef COM_SPI_TMP126_H
#define COM_SPI_TMP126_H

#include <Arduino.h>
#include <SPI.h>
#include "erreur.h"

#define TMP126_PIN_CS    5
#define TMP126_PIN_SCLK  18
#define TMP126_PIN_MOSI  23
#define TMP126_PIN_MISO  19
#define TMP126_SPI_FREQ_HZ  1000000UL

// ---------------------------------------------------------------------------
// Adresses des registres (datasheet TMP126, §8.6 "Register Map")
// ---------------------------------------------------------------------------
enum TMP126_Registre : uint8_t {
    REG_TEMP_RESULT   = 0x00, // R    - résultat de température (14 bits, CA2)
    REG_SLEW_RESULT   = 0x01, // R    - pente de température
    REG_ALERT_STATUS  = 0x02, // R/RC - flags d'alarme (se lit et s'efface)
    REG_CONFIGURATION = 0x03, // R/W  - mode de conversion, période, etc.
    REG_ALERT_ENABLE  = 0x04, // R/W  - active/désactive chaque alarme sur ALERT
    REG_TLOW_LIMIT    = 0x05, // R/W  - seuil bas
    REG_THIGH_LIMIT   = 0x06, // R/W  - seuil haut
    REG_HYSTERESIS    = 0x07, // R/W  - hystérésis haute/basse
    REG_SLEW_LIMIT    = 0x08, // R/W  - seuil de pente
    REG_UNIQUE_ID1    = 0x09, // R    - ID unique (NIST)
    REG_UNIQUE_ID2    = 0x0A,
    REG_UNIQUE_ID3    = 0x0B,
    REG_DEVICE_ID     = 0x0C  // R    - doit valoir 0x2126 (Rev=2h, ID=126h)
};

#define TMP126_CMD_RW_WRITE  (0u << 8)
#define TMP126_CMD_RW_READ   (1u << 8)
#define TMP126_CMD_AUTOINC   (1u << 9)

class ComSpiTMP126 {
public:
    ComSpiTMP126();
    TMP126_Erreur begin();
    void end();
    TMP126_Erreur ecrireRegistre(uint8_t adresse, uint16_t valeur);
    TMP126_Erreur lireRegistre(uint8_t adresse, uint16_t &valeur);

    // Accès en rafale (auto-increment) : utile pour écrire d'un coup toute la
    // configuration (limites, hystérésis, config) au démarrage.
    TMP126_Erreur ecrireRegistresRafale(uint8_t adresseDepart, const uint16_t *valeurs, uint8_t nombre);
    TMP126_Erreur lireRegistresRafale(uint8_t adresseDepart, uint16_t *valeurs, uint8_t nombre);

private:
    SPIClass _spi;
    SPISettings _spiSettings;

    uint16_t transfert16(uint16_t motAEnvoyer);
    void csLow();
    void csHigh();
};

#endif