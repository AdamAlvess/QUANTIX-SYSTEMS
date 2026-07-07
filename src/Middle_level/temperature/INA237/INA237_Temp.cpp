#include "INA237_Temp.h"
#include "I2C_Error.h"

// ═══════════════════════════════════════════════════════════
//  BLOC 2 — Lecture température → conversion en tension (mV)
// ═══════════════════════════════════════════════════════════
float INA237_ReadVoltageFromTemp() {

    // Envoi du pointeur de registre
    Wire.beginTransmission(INA237_ADDR);
    Wire.write(INA237_REG_DIETEMP);
    uint8_t err = Wire.endTransmission(false);

    if (err != 0) {
        Serial.printf("[INA237] Erreur pointeur reg : %s\n",
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

    // Extraction bits [15:4] signés → température °C
    int16_t signed_raw = (int16_t)(raw & 0xFFF0) >> 4;
    float temp_C = signed_raw * 0.125f;

    // Conversion température → tension en mV
    float voltage_mV = temp_C * TEMP_TO_VOLTAGE_FACTOR;

    Serial.printf("[INA237]  Tension : %.2f mV\n",
                   voltage_mV);

    return voltage_mV;
}
