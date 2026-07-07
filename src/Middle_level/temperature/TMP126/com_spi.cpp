#include "com_spi.h"

ComSpiTMP126::ComSpiTMP126()
    : _spi(VSPI),
      _spiSettings(TMP126_SPI_FREQ_HZ, MSBFIRST, SPI_MODE0)
{
}

TMP126_Erreur ComSpiTMP126::begin() {
    pinMode(TMP126_PIN_CS, OUTPUT);
    digitalWrite(TMP126_PIN_CS, HIGH); // CS inactif au repos

    _spi.begin(TMP126_PIN_SCLK, TMP126_PIN_MISO, TMP126_PIN_MOSI, TMP126_PIN_CS);
    delay(2);

    // Vérification de présence : Device_ID doit contenir l'ID 0x126 sur les bits [11:0] (le nibble de poids fort [15:12] est la révision silicium).
    uint16_t deviceId = 0;
    TMP126_Erreur err = lireRegistre(REG_DEVICE_ID, deviceId);
    if (err != TMP126_OK) {
        return err;
    }
    if ((deviceId & 0x0FFF) != 0x126) {
        return ERR_TMP126_DEVICE_ID;
    }

    return TMP126_OK;
}

void ComSpiTMP126::end() {
    _spi.end();
}

void ComSpiTMP126::csLow() {
    digitalWrite(TMP126_PIN_CS, LOW);
}

void ComSpiTMP126::csHigh() {
    digitalWrite(TMP126_PIN_CS, HIGH);
}

uint16_t ComSpiTMP126::transfert16(uint16_t motAEnvoyer) {
    return _spi.transfer16(motAEnvoyer);
}

TMP126_Erreur ComSpiTMP126::ecrireRegistre(uint8_t adresse, uint16_t valeur) {
    const uint16_t motCommande = TMP126_CMD_RW_WRITE | (adresse & 0xFF);

    _spi.beginTransaction(_spiSettings);
    csLow();
    transfert16(motCommande);
    transfert16(valeur);
    csHigh();
    _spi.endTransaction();
    delayMicroseconds(1);

    return TMP126_OK;
}

TMP126_Erreur ComSpiTMP126::lireRegistre(uint8_t adresse, uint16_t &valeur) {
    const uint16_t motCommande = TMP126_CMD_RW_READ | (adresse & 0xFF);

    _spi.beginTransaction(_spiSettings);
    csLow();
    transfert16(motCommande);
    valeur = transfert16(0x0000);
    csHigh();
    _spi.endTransaction();

    delayMicroseconds(1);

    return TMP126_OK;
}

TMP126_Erreur ComSpiTMP126::ecrireRegistresRafale(uint8_t adresseDepart, const uint16_t *valeurs, uint8_t nombre) {
    if (valeurs == nullptr || nombre == 0) {
        return ERR_TMP126_PARAMETRE;
    }

    const uint16_t motCommande = TMP126_CMD_RW_WRITE | TMP126_CMD_AUTOINC | (adresseDepart & 0xFF);

    _spi.beginTransaction(_spiSettings);
    csLow();
    transfert16(motCommande);
    for (uint8_t i = 0; i < nombre; i++) {
        transfert16(valeurs[i]);
    }
    csHigh();
    _spi.endTransaction();

    delayMicroseconds(1);

    return TMP126_OK;
}

TMP126_Erreur ComSpiTMP126::lireRegistresRafale(uint8_t adresseDepart, uint16_t *valeurs, uint8_t nombre) {
    if (valeurs == nullptr || nombre == 0) {
        return ERR_TMP126_PARAMETRE;
    }

    const uint16_t motCommande = TMP126_CMD_RW_READ | TMP126_CMD_AUTOINC | (adresseDepart & 0xFF);

    _spi.beginTransaction(_spiSettings);
    csLow();
    transfert16(motCommande);
    for (uint8_t i = 0; i < nombre; i++) {
        valeurs[i] = transfert16(0x0000);
    }
    csHigh();
    _spi.endTransaction();

    delayMicroseconds(1);

    return TMP126_OK;
}