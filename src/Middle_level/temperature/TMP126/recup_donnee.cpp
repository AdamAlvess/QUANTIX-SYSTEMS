#include "recup_donnee.h"

TMP126_Erreur TMP126::init() {
    TMP126_Erreur err = _com.begin();
    if (err != TMP126_OK) {
        _initialise = false;
        return err;
    }

    // Registre Configuration (0x03) :
    //   - Mode          = 0 -> conversion continue (pas de shutdown)
    //   - One_Shot      = 0
    //   - Int_Comp      = 1 -> mode comparateur (plus simple à exploiter que
    //                          le mode interrupt : le flag colle au dépassement
    //                          réel, sans devoir "acquitter" pour la maj)
    //   - Conv_Period   = 110b (1 Hz, valeur par défaut) -> largement
    //                          suffisant pour une surveillance thermique PCB
    const uint16_t configuration = (1u << 5); // Int_Comp = 1, reste à 0
    err = _com.ecrireRegistre(REG_CONFIGURATION, configuration);
    if (err != TMP126_OK) return err;

    // Registre Alert_Enable (0x04) : on active seuil haut + seuil bas.
    // (CRC_Alert_En reste à sa valeur reset, la pente n'est pas exigée au CDC)
    const uint16_t alertEnable = (1u << 2) | (1u << 1); // THigh_Alert_En | TLow_Alert_En
    err = _com.ecrireRegistre(REG_ALERT_ENABLE, alertEnable);
    if (err != TMP126_OK) return err;

    _initialise = true;
    return TMP126_OK;
}

TMP126_Erreur TMP126::configurerSeuils(float seuilBasC, float seuilHautC, float hysteresisC) {
    if (!_initialise) return ERR_TMP126_NON_INITIALISE;
    if (seuilHautC <= seuilBasC) return ERR_TMP126_PARAMETRE; // exigé par la datasheet

    TMP126_Erreur err;

    err = _com.ecrireRegistre(REG_TLOW_LIMIT, celsiusVers14Bits(seuilBasC));
    if (err != TMP126_OK) return err;

    err = _com.ecrireRegistre(REG_THIGH_LIMIT, celsiusVers14Bits(seuilHautC));
    if (err != TMP126_OK) return err;

    // Hystérésis : 8 bits non signés, LSB = 0.5°C, même valeur haute et basse.
    const uint8_t hystBrut = (uint8_t)(hysteresisC / 0.5f);
    const uint16_t hystereseReg = ((uint16_t)hystBrut << 8) | hystBrut;
    err = _com.ecrireRegistre(REG_HYSTERESIS, hystereseReg);
    if (err != TMP126_OK) return err;

    return TMP126_OK;
}

TMP126_Erreur TMP126::lireMesure(TMP126_Mesure &mesure) {
    if (!_initialise) return ERR_TMP126_NON_INITIALISE;

    uint16_t tempBrute = 0;
    TMP126_Erreur err = _com.lireRegistre(REG_TEMP_RESULT, tempBrute);
    if (err != TMP126_OK) return err;

    uint16_t statutBrut = 0;
    err = _com.lireRegistre(REG_ALERT_STATUS, statutBrut);
    if (err != TMP126_OK) return err;

    mesure.temperature_c = brut14VersCelsius(tempBrute);
    mesure.donnee_prete  = statutBrut & (1u << 0); // Data_Ready_Flag
    mesure.alerte_haute  = statutBrut & (1u << 2); // THigh_Flag
    mesure.alerte_basse  = statutBrut & (1u << 1); // TLow_Flag
    mesure.alerte_pente  = statutBrut & (1u << 5); // Slew_Flag
    mesure.erreur_crc    = statutBrut & (1u << 7); // CRC_Flag
    mesure.horodatage_ms = millis();

    return TMP126_OK;
}

float TMP126::brut14VersCelsius(uint16_t brut) {
    // Temp_Result[13:0] occupe les bits [15:2] du registre, en complément à 2,
    // LSB = 0.03125 °C (datasheet §8.6.1). On décale à droite de 2 en
    // interprétant la valeur comme signée pour récupérer le bon signe.
    int16_t valeurSignee = (int16_t)brut;
    valeurSignee >>= 2; // décalage arithmétique (compilateur ESP32 = gcc/xtensa)
    return (float)valeurSignee * 0.03125f;
}

uint16_t TMP126::celsiusVers14Bits(float temperatureC) {
    const int16_t valeur14 = (int16_t)(temperatureC / 0.03125f);
    return ((uint16_t)valeur14) << 2;
}