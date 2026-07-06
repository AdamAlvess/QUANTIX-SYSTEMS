#include "groove.h"

GrooveErreur GrooveI2C::begin() {
    bool ok = Wire.begin(GROOVE_I2C_SDA, GROOVE_I2C_SCL, GROOVE_I2C_FREQ_HZ);
    if (!ok) {
        _initialise = false;
        return ERR_GROOVE_TRANSFERT;
    }

    _initialise = true;
    return GROOVE_OK;
}

GrooveErreur GrooveI2C::verifierAdresseDisponible(uint8_t adresse7bits) {
    if (adresse7bits < 0x08 || adresse7bits > 0x77) {
        return ERR_GROOVE_ADRESSE_INVALIDE;
    }

    for (uint8_t i = 0; i < GROOVE_NB_ADRESSES_RESERVEES; i++) {
        if (GROOVE_ADRESSES_BUS_PARTAGE[i].adresse7bits == adresse7bits) {
            return ERR_GROOVE_ADRESSE_CONFLIT;
        }
    }

    return GROOVE_OK;
}

bool GrooveI2C::capteurPresent(uint8_t adresse7bits) {
    if (!_initialise) return false;

    Wire.beginTransmission(adresse7bits);
    uint8_t resultat = Wire.endTransmission();
    return (resultat == 0); // 0 = ACK reçu -> un device répond à cette adresse
}

GrooveErreur GrooveI2C::scannerBus(uint8_t *adressesTrouvees, uint8_t tailleMax, uint8_t &nombreTrouve) {
    if (!_initialise) return ERR_GROOVE_NON_INITIALISE;
    if (adressesTrouvees == nullptr || tailleMax == 0) return ERR_GROOVE_ADRESSE_INVALIDE;

    nombreTrouve = 0;
    for (uint8_t adresse = 0x08; adresse <= 0x77; adresse++) {
        if (capteurPresent(adresse)) {
            if (nombreTrouve < tailleMax) {
                adressesTrouvees[nombreTrouve] = adresse;
            }
            nombreTrouve++;
        }
    }

    return GROOVE_OK;
}

GrooveErreur GrooveI2C::lireRegistre8(uint8_t adresse7bits, uint8_t reg, uint8_t &valeur) {
    if (!_initialise) return ERR_GROOVE_NON_INITIALISE;

    Wire.beginTransmission(adresse7bits);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) { // restart, pas de stop
        return ERR_GROOVE_CAPTEUR_ABSENT;
    }

    uint8_t recu = Wire.requestFrom((int)adresse7bits, 1);
    if (recu != 1 || Wire.available() < 1) {
        return ERR_GROOVE_TRANSFERT;
    }

    valeur = Wire.read();
    return GROOVE_OK;
}

GrooveErreur GrooveI2C::ecrireRegistre8(uint8_t adresse7bits, uint8_t reg, uint8_t valeur) {
    if (!_initialise) return ERR_GROOVE_NON_INITIALISE;

    Wire.beginTransmission(adresse7bits);
    Wire.write(reg);
    Wire.write(valeur);
    if (Wire.endTransmission(true) != 0) {
        return ERR_GROOVE_CAPTEUR_ABSENT;
    }

    return GROOVE_OK;
}

GrooveErreur GrooveI2C::lireRegistre16(uint8_t adresse7bits, uint8_t reg, uint16_t &valeur, bool msbFirst) {
    if (!_initialise) return ERR_GROOVE_NON_INITIALISE;

    Wire.beginTransmission(adresse7bits);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        return ERR_GROOVE_CAPTEUR_ABSENT;
    }

    uint8_t recu = Wire.requestFrom((int)adresse7bits, 2);
    if (recu != 2 || Wire.available() < 2) {
        return ERR_GROOVE_TRANSFERT;
    }

    uint8_t octetA = Wire.read();
    uint8_t octetB = Wire.read();

    valeur = msbFirst ? ((uint16_t)octetA << 8) | octetB
                      : ((uint16_t)octetB << 8) | octetA;

    return GROOVE_OK;
}