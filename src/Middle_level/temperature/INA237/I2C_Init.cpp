#include "I2C_Init.h"
#include "I2C_Error.h"

// ═══════════════════════════════════════════════════════════
//  BLOC 1 — Initialisation de la communication I2C
// ═══════════════════════════════════════════════════════════
bool I2C_Init(uint8_t sda, uint8_t scl, uint32_t freq) {

    Wire.begin(sda, scl);
    Wire.setClock(freq);
    Wire.setTimeOut(100);

    Serial.printf("[I2C_Init] Bus démarré – SDA:%d SCL:%d @ %lu Hz\n",
                  sda, scl, freq);

    // Vérification présence INA237
    Wire.beginTransmission(INA237_ADDR);
    uint8_t err = Wire.endTransmission();

    if (err != 0) {
        Serial.printf("[I2C_Init] INA237 non détecté : %s\n", I2C_GetErrorStr(err));
        return false;
    }

    // Reset logiciel
    Wire.beginTransmission(INA237_ADDR);
    Wire.write(INA237_REG_CONFIG);
    Wire.write(0x80);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(5);

    Serial.println("[I2C_Init] Initialisation OK");
    return true;
}
