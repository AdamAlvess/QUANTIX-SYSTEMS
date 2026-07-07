#include "MikroBus.h"

// ═══════════════════════════════════════════════════════════
//  Configuration des GPIO du connecteur MikroBus
//  selon le module branché
// ═══════════════════════════════════════════════════════════
bool MikroBus_InitGPIO(MikroBusModule module) {
    Serial.println("[MikroBus] Configuration des GPIO...");

    // ── Pins communs à tous les modules ──────────────────
    pinMode(MIKROBUS_RST_PIN, OUTPUT);
    digitalWrite(MIKROBUS_RST_PIN, HIGH);   // Pas en reset

    pinMode(MIKROBUS_INT_PIN, INPUT);        // Interruption en entrée
    pinMode(MIKROBUS_AN_PIN,  INPUT);        // Analogique en entrée

    // ── Pins spécifiques selon le module ─────────────────
    switch (module) {

        case MikroBusModule::LORA_RN2483:
            // RN2483 utilise UART → TX/RX configurés par HardwareSerial
            // CS, SPI, I2C non utilisés → mis en haute impédance
            pinMode(MIKROBUS_CS_PIN,   INPUT);
            pinMode(MIKROBUS_SCK_PIN,  INPUT);
            pinMode(MIKROBUS_MISO_PIN, INPUT);
            pinMode(MIKROBUS_MOSI_PIN, INPUT);
            pinMode(MIKROBUS_PWM_PIN,  INPUT);
            Serial.println("[MikroBus] Mode UART (LoRa RN2483) configuré");
            break;

        case MikroBusModule::NONE:
        default:
            // Aucun module → tous les pins en haute impédance
            pinMode(MIKROBUS_CS_PIN,   INPUT);
            pinMode(MIKROBUS_SCK_PIN,  INPUT);
            pinMode(MIKROBUS_MISO_PIN, INPUT);
            pinMode(MIKROBUS_MOSI_PIN, INPUT);
            pinMode(MIKROBUS_PWM_PIN,  INPUT);
            Serial.println("[MikroBus] Aucun module – GPIO en haute impédance");
            break;
    }

    MikroBus_PrintConfig();
    return true;
}

// ─────────────────────────────────────────────
//  Reset matériel du module branché
// ─────────────────────────────────────────────
void MikroBus_Reset() {
    Serial.println("[MikroBus] Reset module...");
    digitalWrite(MIKROBUS_RST_PIN, LOW);
    delay(100);
    digitalWrite(MIKROBUS_RST_PIN, HIGH);
    delay(500);
    Serial.println("[MikroBus] Reset terminé");
}

// ─────────────────────────────────────────────
//  Affichage de la configuration courante
// ─────────────────────────────────────────────
void MikroBus_PrintConfig() {
    Serial.println("┌─────────────────────────────────┐");
    Serial.println("│     MikroBus GPIO Config         │");
    Serial.printf ("│ AN   → GPIO%-2d  [INPUT]           │\n", MIKROBUS_AN_PIN);
    Serial.printf ("│ RST  → GPIO%-2d  [OUTPUT]          │\n", MIKROBUS_RST_PIN);
    Serial.printf ("│ CS   → GPIO%-2d  [INPUT]           │\n", MIKROBUS_CS_PIN);
    Serial.printf ("│ SCK  → GPIO%-2d  [INPUT]           │\n", MIKROBUS_SCK_PIN);
    Serial.printf ("│ MISO → GPIO%-2d  [INPUT]           │\n", MIKROBUS_MISO_PIN);
    Serial.printf ("│ MOSI → GPIO%-2d  [INPUT]           │\n", MIKROBUS_MOSI_PIN);
    Serial.printf ("│ PWM  → GPIO%-2d  [INPUT]           │\n", MIKROBUS_PWM_PIN);
    Serial.printf ("│ INT  → GPIO%-2d  [INPUT]           │\n", MIKROBUS_INT_PIN);
    Serial.printf ("│ RX   → GPIO%-2d  [UART]            │\n", MIKROBUS_RX_PIN);
    Serial.printf ("│ TX   → GPIO%-2d  [UART]            │\n", MIKROBUS_TX_PIN);
    Serial.printf ("│ SCL  → GPIO%-2d  [I2C]             │\n", MIKROBUS_SCL_PIN);
    Serial.printf ("│ SDA  → GPIO%-2d  [I2C]             │\n", MIKROBUS_SDA_PIN);
    Serial.println("└─────────────────────────────────┘");
}
