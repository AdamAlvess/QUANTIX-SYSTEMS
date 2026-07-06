/**
 * ╔══════════════════════════════════════════════════════════╗
 * ║   ESP32-PICO-D4 – Carte de développement YNOV 2026       ║
 * ║   Module : Initialisation & Sécurité du bus I2C          ║
 * ║   Composants : INA237AIDGST, connecteurs Grove           ║
 * ╚══════════════════════════════════════════════════════════╝
 */

#include <Arduino.h>
#include "I2C_Manager.h"
#include "INA237.h"

// ─────────────────────────────────────────────
//  Périphériques
// ─────────────────────────────────────────────
INA237 ina237(INA237_DEFAULT_ADDR);   // A0=GND A1=GND → 0x40

// ─────────────────────────────────────────────
//  Intervalle de lecture (ms)
// ─────────────────────────────────────────────
static constexpr uint32_t READ_INTERVAL_MS = 1000;

// ─────────────────────────────────────────────
//  Compteur d'erreurs consécutives
//  → Si trop d'erreurs : tentative de recovery
// ─────────────────────────────────────────────
static uint8_t  s_errorCount    = 0;
static uint32_t s_lastReadMs    = 0;
static constexpr uint8_t MAX_CONSECUTIVE_ERRORS = 5;

// ─────────────────────────────────────────────
//  Prototypes
// ─────────────────────────────────────────────
static bool initI2CBus();
static bool initSensors();
static void handleI2CError();
static void taskReadSensors();

// ═════════════════════════════════════════════
//  SETUP
// ═════════════════════════════════════════════
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    delay(200);

    Serial.println("\n╔══════════════════════════════╗");
    Serial.println("║  ESP32 I2C – Boot & Sécurité ║");
    Serial.println("╚══════════════════════════════╝\n");

    // 1. Initialisation du bus I2C
    if (!initI2CBus()) {
        Serial.println("[FATAL] Bus I2C non initialisé. Arrêt.");
        while (true) delay(1000);
    }

    // 2. Scan complet (debug)
    I2CManager::scan(Serial);

    // 3. Initialisation des capteurs
    if (!initSensors()) {
        Serial.println("[WARN] Certains capteurs non disponibles.");
    }

    Serial.println("\n[SYSTEM] Démarrage de la boucle de mesure...\n");
}

// ═════════════════════════════════════════════
//  LOOP
// ═════════════════════════════════════════════
void loop() {
    uint32_t now = millis();

    if (now - s_lastReadMs >= READ_INTERVAL_MS) {
        s_lastReadMs = now;
        taskReadSensors();
    }
}

// ─────────────────────────────────────────────
//  Initialisation du bus I2C avec retry
// ─────────────────────────────────────────────
static bool initI2CBus() {
    for (uint8_t attempt = 1; attempt <= 3; attempt++) {
        Serial.printf("[I2C] Init tentative %d/3...\n", attempt);
        if (I2CManager::begin()) return true;
        delay(100);
    }
    return false;
}

// ─────────────────────────────────────────────
//  Initialisation des capteurs I2C
// ─────────────────────────────────────────────
static bool initSensors() {
    bool ok = true;

    // INA237 – mesure courant & température
    if (!ina237.begin()) {
        Serial.println("[WARN] INA237 non initialisé");
        ok = false;
    }

    // Ajouter ici d'autres capteurs Grove si nécessaire
    // ex: Grove Temp → TMP116, Grove IMU → LSM6DS3...

    return ok;
}

// ─────────────────────────────────────────────
//  Lecture périodique des capteurs
// ─────────────────────────────────────────────
static void taskReadSensors() {
    INA237Data data;

    if (ina237.readAll(data)) {
        s_errorCount = 0;  // Reset compteur d'erreurs
        ina237.printData(data, Serial);

        // ── Alertes de sécurité ──────────────────
        if (data.busVoltage_V > 36.0f) {
            Serial.println("[ALERTE] Tension bus > 36V !");
        }
        if (data.current_A > INA237_MAX_CURRENT_A * 0.9f) {
            Serial.printf("[ALERTE] Courant élevé : %.2f A\n", data.current_A);
        }
        if (data.temperature_C > 85.0f) {
            Serial.printf("[ALERTE] Température critique : %.1f °C\n",
                          data.temperature_C);
        }
        if (data.busVoltage_V < 3.0f && data.busVoltage_V > 0.1f) {
            Serial.println("[ALERTE] Tension bus faible (< 3V)");
        }

    } else {
        s_errorCount++;
        Serial.printf("[ERR] Lecture INA237 échouée (%d/%d)\n",
                      s_errorCount, MAX_CONSECUTIVE_ERRORS);

        if (s_errorCount >= MAX_CONSECUTIVE_ERRORS) {
            handleI2CError();
        }
    }
}

// ─────────────────────────────────────────────
//  Gestion des erreurs répétées → recovery
// ─────────────────────────────────────────────
static void handleI2CError() {
    Serial.println("[I2C] Trop d'erreurs – tentative de recovery...");
    s_errorCount = 0;

    if (I2CManager::recovery()) {
        Serial.println("[I2C] Recovery OK – réinitialisation des capteurs");
        delay(100);
        initSensors();
    } else {
        Serial.println("[I2C] Recovery ECHEC – vérifier le câblage");
        // Ici on pourrait déclencher un reset matériel via GPIO RESET
        // ou notifier via BLE / LED d'erreur
    }
}
