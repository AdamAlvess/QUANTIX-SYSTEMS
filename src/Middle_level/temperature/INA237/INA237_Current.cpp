#include "INA237_Current.h"
#include "I2C_Error.h"

// ─────────────────────────────────────────────
//  Helpers lecture/écriture registre 16 bits
// ─────────────────────────────────────────────
static bool writeReg(uint8_t reg, uint16_t val) {
    Wire.beginTransmission(INA237_ADDR);
    Wire.write(reg);
    Wire.write((uint8_t)(val >> 8));
    Wire.write((uint8_t)(val & 0xFF));
    return (Wire.endTransmission() == 0);
}

static bool readReg(uint8_t reg, uint16_t &val) {
    Wire.beginTransmission(INA237_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom((uint8_t)INA237_ADDR, (uint8_t)2) != 2) return false;

    val = ((uint16_t)Wire.read() << 8) | Wire.read();
    return true;
}

// ═══════════════════════════════════════════════════════════
//  Initialisation INA237 pour mesure de courant
// ═══════════════════════════════════════════════════════════
bool INA237_Current_Init() {
    Serial.println("[INA237_Current] Initialisation...");

    // Vérification présence
    Wire.beginTransmission(INA237_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.println("[INA237_Current] Composant non détecté");
        return false;
    }

    // Reset logiciel
    if (!writeReg(INA237_REG_CONFIG, 0x8000)) {
        Serial.println("[INA237_Current] Erreur reset");
        return false;
    }
    delay(5);

    // Config : ADCRANGE = 1 (±40.96mV), mode continu
    if (!writeReg(INA237_REG_CONFIG, 0x0010)) {
        Serial.println("[INA237_Current] Erreur config");
        return false;
    }

    // ADC config : mode continu courant + tension + temp
    // VBUSCT=4 (1.052ms), VSHCT=4, VTCT=4, AVG=0
    if (!writeReg(INA237_REG_ADC_CONFIG, 0xFB68)) {
        Serial.println("[INA237_Current] Erreur ADC config");
        return false;
    }

    // Calibration shunt
    Serial.printf("[INA237_Current] SHUNT_CAL = %u  |  LSB = %.7f A\n",
                  INA237_SHUNT_CAL_VAL, INA237_CURRENT_LSB);

    if (!writeReg(INA237_REG_SHUNT_CAL, INA237_SHUNT_CAL_VAL)) {
        Serial.println("[INA237_Current] Erreur SHUNT_CAL");
        return false;
    }

    Serial.println("[INA237_Current] Init OK");
    return true;
}

// ═══════════════════════════════════════════════════════════
//  Lecture courant, tension bus, tension shunt, puissance
// ═══════════════════════════════════════════════════════════
bool INA237_Current_Read(INA237Measure &out) {
    out.valid = false;

    uint16_t raw = 0;

    // ── Courant ──────────────────────────────────────────
    if (!readReg(INA237_REG_CURRENT, raw)) {
        Serial.println("[INA237_Current] Erreur lecture courant");
        return false;
    }
    out.current_A = (int16_t)raw * INA237_CURRENT_LSB;

    // ── Tension bus (LSB = 3.125 mV) ─────────────────────
    if (!readReg(INA237_REG_BUS_V, raw)) {
        Serial.println("[INA237_Current] Erreur lecture tension bus");
        return false;
    }
    out.busVoltage_V = (int16_t)raw * 3.125e-3f;

    // ── Tension shunt (LSB = 1.25 µV avec ADCRANGE=1) ────
    if (!readReg(INA237_REG_SHUNT_V, raw)) {
        Serial.println("[INA237_Current] Erreur lecture tension shunt");
        return false;
    }
    out.shuntVoltage_mV = (int16_t)raw * 1.25e-6f * 1000.0f;

    // ── Puissance calculée ────────────────────────────────
    out.power_W = out.busVoltage_V * out.current_A;

    out.valid = true;
    return true;
}

// ═══════════════════════════════════════════════════════════
//  Affichage formaté
// ═══════════════════════════════════════════════════════════
void INA237_Current_Print(const INA237Measure &m) {
    if (!m.valid) {
        Serial.println("[INA237_Current] Mesure invalide");
        return;
    }
    Serial.println("┌──────────────────────────────────┐");
    Serial.printf ("│ Courant       : %8.4f A        │\n", m.current_A);
    Serial.printf ("│ Tension bus   : %8.4f V        │\n", m.busVoltage_V);
    Serial.printf ("│ Tension shunt : %8.4f mV       │\n", m.shuntVoltage_mV);
    Serial.printf ("│ Puissance     : %8.4f W        │\n", m.power_W);
    Serial.println("└──────────────────────────────────┘");
}
