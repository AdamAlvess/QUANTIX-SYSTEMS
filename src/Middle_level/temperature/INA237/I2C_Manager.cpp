#include "I2C_Manager.h"

// ─────────────────────────────────────────────
//  Initialisation du bus
// ─────────────────────────────────────────────
bool I2CManager::begin() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(I2C_FREQ_HZ);
    Wire.setTimeOut(I2C_TIMEOUT_MS);
    Serial.printf("[I2C] Bus initialisé – SDA:%d SCL:%d @ %d Hz\n",
                  I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    return true;
}

// ─────────────────────────────────────────────
//  Vérification de présence d'un composant
// ─────────────────────────────────────────────
bool I2CManager::isDevicePresent(uint8_t addr) {
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
}

// ─────────────────────────────────────────────
//  Scan de toutes les adresses (debug)
// ─────────────────────────────────────────────
void I2CManager::scan(Stream &serial) {
    serial.println("[I2C] Scan du bus...");
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        if (isDevicePresent(addr)) {
            serial.printf("  → Composant trouvé : 0x%02X\n", addr);
            found++;
        }
    }
    if (found == 0) serial.println("  Aucun composant détecté !");
    serial.printf("[I2C] Scan terminé – %d composant(s)\n", found);
}

// ─────────────────────────────────────────────
//  Recovery : 9 coups d'horloge manuels
//  (débloque un esclave coincé à l'état bas)
// ─────────────────────────────────────────────
bool I2CManager::recovery() {
    Serial.println("[I2C] Recovery : envoi de 9 pulses SCL...");

    Wire.end();

    pinMode(I2C_SCL_PIN, OUTPUT);
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);

    for (int i = 0; i < 9; i++) {
        digitalWrite(I2C_SCL_PIN, HIGH);
        delayMicroseconds(5);
        digitalWrite(I2C_SCL_PIN, LOW);
        delayMicroseconds(5);
    }

    // STOP condition manuelle
    pinMode(I2C_SDA_PIN, OUTPUT);
    digitalWrite(I2C_SDA_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL_PIN, HIGH);
    delayMicroseconds(5);
    digitalWrite(I2C_SDA_PIN, HIGH);
    delayMicroseconds(5);

    // Ré-init propre du driver
    begin();

    bool ok = !isDevicePresent(0x00); // bus libre si pas de réponse au broadcast
    Serial.printf("[I2C] Recovery %s\n", ok ? "OK" : "ECHEC");
    return ok;
}

// ─────────────────────────────────────────────
//  Écriture dans un registre
// ─────────────────────────────────────────────
I2CError I2CManager::writeReg(uint8_t addr, uint8_t reg,
                               const uint8_t *data, size_t len) {
    for (uint8_t attempt = 0; attempt < I2C_MAX_RETRIES; attempt++) {
        Wire.beginTransmission(addr);
        Wire.write(reg);
        Wire.write(data, len);
        uint8_t err = Wire.endTransmission(true);

        if (err == 0) return I2CError::OK;

        Serial.printf("[I2C] writeReg 0x%02X reg=0x%02X err=%d (tentative %d/%d)\n",
                      addr, reg, err, attempt + 1, I2C_MAX_RETRIES);
        delay(10);
    }
    return mapWireError(Wire.endTransmission());
}

// ─────────────────────────────────────────────
//  Lecture depuis un registre
// ─────────────────────────────────────────────
I2CError I2CManager::readReg(uint8_t addr, uint8_t reg,
                              uint8_t *data, size_t len) {
    for (uint8_t attempt = 0; attempt < I2C_MAX_RETRIES; attempt++) {
        // Envoi du pointeur de registre
        Wire.beginTransmission(addr);
        Wire.write(reg);
        uint8_t err = Wire.endTransmission(false); // Repeated START

        if (err != 0) {
            Serial.printf("[I2C] readReg pointer 0x%02X reg=0x%02X err=%d\n",
                          addr, reg, err);
            delay(10);
            continue;
        }

        // Lecture des octets
        uint8_t received = Wire.requestFrom((int)addr, (int)len, true);
        if (received != len) {
            Serial.printf("[I2C] readReg : attendu %u octets, reçu %u\n",
                          len, received);
            delay(10);
            continue;
        }

        for (size_t i = 0; i < len; i++) {
            data[i] = Wire.read();
        }
        return I2CError::OK;
    }
    return I2CError::TIMEOUT;
}

// ─────────────────────────────────────────────
//  Mapping code Wire → I2CError
// ─────────────────────────────────────────────
I2CError I2CManager::mapWireError(uint8_t code) {
    switch (code) {
        case 0: return I2CError::OK;
        case 1: return I2CError::DATA_TOO_LONG;
        case 2: return I2CError::NACK_ADDR;
        case 3: return I2CError::NACK_DATA;
        case 5: return I2CError::TIMEOUT;
        default: return I2CError::OTHER;
    }
}

const char* I2CManager::errorStr(I2CError err) {
    switch (err) {
        case I2CError::OK:            return "OK";
        case I2CError::DATA_TOO_LONG: return "Données trop longues";
        case I2CError::NACK_ADDR:     return "NACK adresse";
        case I2CError::NACK_DATA:     return "NACK données";
        case I2CError::TIMEOUT:       return "Timeout";
        case I2CError::BUS_BUSY:      return "Bus occupé";
        default:                      return "Erreur inconnue";
    }
}
