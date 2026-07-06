#include "INA237_Temp.h"
#include "I2C_Error.h"

// ═══════════════════════════════════════════════════════════
//  BLOC 2 — Lecture du registre de température interne
//  Registre DIETEMP (0x06)
//  Résolution : 125 m°C / LSB — bits [15:4] signés
// ═══════════════════════════════════════════════════════════
float INA237_ReadTemperature() {

    // Envoi du pointeur de registre
    Wire.beginTransmission(INA237_ADDR);
    Wire.write(INA237_REG_DIETEMP);
    uint8_t err = Wire.endTransmission(false);   // Repeated START

    if (err != 0) {
        Serial.printf("[INA237_Temp] Erreur pointeur reg : %s\n",
                      I2C_GetErrorStr(err));
        return NAN;
    }

    // Lecture 2 octets
    uint8_t received = Wire.requestFrom((uint8_t)INA237_ADDR, (uint8_t)2);
    if (received != 2) {
        Serial.println("[INA237_Temp] Erreur : octets insuffisants");
        return NAN;
    }

    uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();

    // Extraction bits [15:4] signés → conversion °C
    int16_t signed_raw = (int16_t)(raw & 0xFFF0) >> 4;
    float temp = signed_raw * 0.125f;

    Serial.printf("[INA237_Temp] Température : %.2f °C\n", temp);
    return temp;
}
