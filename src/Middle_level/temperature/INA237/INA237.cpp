#include "INA237.h"

// ─────────────────────────────────────────────
//  Constructeur
// ─────────────────────────────────────────────
INA237::INA237(uint8_t addr) : _addr(addr) {}

// ─────────────────────────────────────────────
//  Initialisation complète
// ─────────────────────────────────────────────
bool INA237::begin() {
    if (!I2CManager::isDevicePresent(_addr)) {
        Serial.printf("[INA237] Composant non trouvé à l'adresse 0x%02X\n", _addr);
        return false;
    }
    Serial.printf("[INA237] Trouvé à 0x%02X\n", _addr);

    if (!verifyId())   return false;
    if (!reset())      return false;
    delay(5);
    if (!configure())  return false;

    Serial.println("[INA237] Initialisation OK");
    return true;
}

// ─────────────────────────────────────────────
//  Reset logiciel
// ─────────────────────────────────────────────
bool INA237::reset() {
    if (!writeReg16(INA237_REG_CONFIG, INA237_CONFIG_RESET)) {
        Serial.println("[INA237] Erreur reset");
        return false;
    }
    delay(2);
    return true;
}

// ─────────────────────────────────────────────
//  Vérification du Device ID (0x2370 attendu)
// ─────────────────────────────────────────────
bool INA237::verifyId() {
    uint16_t devId = 0;
    if (!readReg16(INA237_REG_DEVICE_ID, devId)) {
        Serial.println("[INA237] Lecture Device ID échouée");
        return false;
    }
    uint16_t chipId = (devId >> 4) & 0x0FFF;
    Serial.printf("[INA237] Device ID = 0x%04X (chip = 0x%03X)\n", devId, chipId);

    if (chipId != 0x237) {
        Serial.printf("[INA237] ATTENTION : ID inattendu (0x%03X)\n", chipId);
        // On continue quand même (certaines révisions varient)
    }
    return true;
}

// ─────────────────────────────────────────────
//  Configuration ADC + calibration shunt
// ─────────────────────────────────────────────
bool INA237::configure() {
    // Config : plage ±40.96 mV pour meilleure précision
    if (!writeReg16(INA237_REG_CONFIG, INA237_ADC_RANGE_LOW)) {
        Serial.println("[INA237] Erreur config registre CONFIG");
        return false;
    }

    // ADC : mode continu, temps conversion 1 ms, 1 échantillon
    if (!writeReg16(INA237_REG_ADC_CONFIG, INA237_ADC_CONFIG_DEFAULT)) {
        Serial.println("[INA237] Erreur config registre ADC_CONFIG");
        return false;
    }

    // Calibration shunt
    Serial.printf("[INA237] SHUNT_CAL = %u  |  LSB = %.6f A\n",
                  INA237_SHUNT_CAL_VAL, INA237_CURRENT_LSB_A);
    if (!writeReg16(INA237_REG_SHUNT_CAL, INA237_SHUNT_CAL_VAL)) {
        Serial.println("[INA237] Erreur écriture SHUNT_CAL");
        return false;
    }

    return true;
}

// ─────────────────────────────────────────────
//  Lecture complète de toutes les grandeurs
// ─────────────────────────────────────────────
bool INA237::readAll(INA237Data &out) {
    out.valid = false;

    out.busVoltage_V   = readBusVoltage();
    out.shuntVoltage_mV = readShuntVoltage();
    out.current_A      = readCurrent();
    out.power_W        = readPower();
    out.temperature_C  = readTemperature();

    // Validation basique
    if (isnan(out.busVoltage_V) || isnan(out.current_A)) {
        Serial.println("[INA237] Données invalides");
        return false;
    }
    out.valid = true;
    return true;
}

// ─────────────────────────────────────────────
//  Tension bus (LSB = 3.125 mV)
// ─────────────────────────────────────────────
float INA237::readBusVoltage() {
    uint16_t raw = 0;
    if (!readReg16(INA237_REG_BUS_V, raw)) return NAN;
    // Bit 0 est réservé → shift right 0 (registre aligné à droite sur 16 bits)
    return (int16_t)raw * 3.125e-3f;  // 3.125 mV / LSB
}

// ─────────────────────────────────────────────
//  Tension shunt (LSB dépend de ADCRANGE)
//  ADCRANGE=1 → ±40.96 mV → LSB = 1.25 µV
// ─────────────────────────────────────────────
float INA237::readShuntVoltage() {
    uint16_t raw = 0;
    if (!readReg16(INA237_REG_SHUNT_V, raw)) return NAN;
    int16_t signed_raw = toSigned16(raw);
    return signed_raw * 1.25e-6f * 1000.0f; // → mV
}

// ─────────────────────────────────────────────
//  Courant (LSB = CURRENT_LSB_A)
// ─────────────────────────────────────────────
float INA237::readCurrent() {
    uint16_t raw = 0;
    if (!readReg16(INA237_REG_CURRENT, raw)) return NAN;
    return toSigned16(raw) * INA237_CURRENT_LSB_A;
}

// ─────────────────────────────────────────────
//  Puissance (LSB = 0.2 * CURRENT_LSB_A)
// ─────────────────────────────────────────────
float INA237::readPower() {
    uint16_t raw = 0;
    if (!readReg16(INA237_REG_POWER, raw)) return NAN;
    // Registre Power sur 24 bits → on lit 16 bits ici (simplification)
    // Pour 24 bits complets il faudrait une lecture sur 3 octets
    return raw * 0.2f * INA237_CURRENT_LSB_A;
}

// ─────────────────────────────────────────────
//  Température interne (LSB = 125 m°C)
// ─────────────────────────────────────────────
float INA237::readTemperature() {
    uint16_t raw = 0;
    if (!readReg16(INA237_REG_DIETEMP, raw)) return NAN;
    // Bits [15:4] signés, bit [3:0] réservés
    int16_t signed_raw = (int16_t)(raw & 0xFFF0) >> 4;
    return signed_raw * 0.125f;  // 125 m°C / LSB
}

// ─────────────────────────────────────────────
//  Affichage formaté sur Serial
// ─────────────────────────────────────────────
void INA237::printData(const INA237Data &d, Stream &serial) {
    if (!d.valid) {
        serial.println("[INA237] Données non valides");
        return;
    }
    serial.println("┌─────────────────────────────┐");
    serial.printf ("│ Tension bus   : %7.3f V    │\n", d.busVoltage_V);
    serial.printf ("│ Tension shunt : %7.3f mV   │\n", d.shuntVoltage_mV);
    serial.printf ("│ Courant       : %7.3f A    │\n", d.current_A);
    serial.printf ("│ Puissance     : %7.3f W    │\n", d.power_W);
    serial.printf ("│ Température   : %7.2f °C   │\n", d.temperature_C);
    serial.println("└─────────────────────────────┘");
}

// ─────────────────────────────────────────────
//  Helpers privés
// ─────────────────────────────────────────────
bool INA237::writeReg16(uint8_t reg, uint16_t val) {
    uint8_t buf[2] = { (uint8_t)(val >> 8), (uint8_t)(val & 0xFF) };
    I2CError err = I2CManager::writeReg(_addr, reg, buf, 2);
    if (err != I2CError::OK) {
        Serial.printf("[INA237] writeReg16 0x%02X : %s\n",
                      reg, I2CManager::errorStr(err));
        return false;
    }
    return true;
}

bool INA237::readReg16(uint8_t reg, uint16_t &val) {
    uint8_t buf[2] = {0, 0};
    I2CError err = I2CManager::readReg(_addr, reg, buf, 2);
    if (err != I2CError::OK) {
        Serial.printf("[INA237] readReg16 0x%02X : %s\n",
                      reg, I2CManager::errorStr(err));
        return false;
    }
    val = ((uint16_t)buf[0] << 8) | buf[1];  // Big-endian
    return true;
}

int16_t INA237::toSigned16(uint16_t raw) {
    return (int16_t)raw;
}
