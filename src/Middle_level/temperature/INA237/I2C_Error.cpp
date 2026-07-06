#include "I2C_Error.h"
#include "I2C_Init.h"

// ═══════════════════════════════════════════════════════════
//  BLOC 3 — Gestion des erreurs de communication
// ═══════════════════════════════════════════════════════════

// Traduit le code Wire en message lisible
const char* I2C_GetErrorStr(uint8_t wireCode) {
    switch (wireCode) {
        case 0: return "Aucune erreur";
        case 2: return "NACK adresse – composant absent ?";
        case 3: return "NACK données – transmission échouée";
        case 5: return "Timeout – bus bloqué ?";
        default: return "Erreur inconnue";
    }
}

// Recovery : 9 pulses SCL manuels pour débloquer le bus
bool I2C_Recover(uint8_t sda, uint8_t scl) {
    Serial.println("[I2C_Error] Recovery – envoi de 9 pulses SCL...");

    Wire.end();

    pinMode(scl, OUTPUT);
    pinMode(sda, INPUT_PULLUP);

    for (int i = 0; i < 9; i++) {
        digitalWrite(scl, HIGH); delayMicroseconds(5);
        digitalWrite(scl, LOW);  delayMicroseconds(5);
    }

    // STOP condition manuelle
    pinMode(sda, OUTPUT);
    digitalWrite(sda, LOW);  delayMicroseconds(5);
    digitalWrite(scl, HIGH); delayMicroseconds(5);
    digitalWrite(sda, HIGH); delayMicroseconds(5);

    bool ok = I2C_Init(sda, scl);
    Serial.printf("[I2C_Error] Recovery %s\n", ok ? "réussie" : "échouée");
    return ok;
}
