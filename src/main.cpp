#include <Arduino.h>
#include "Middle_level/temperature/INA237/I2C_Init.h"
#include "Middle_level/temperature/INA237/INA237_Temp.h"
#include "Middle_level/temperature/INA237/INA237_Current.h"
#include "Middle_level/temperature/INA237/I2C_Error.h"

static uint8_t  s_errCount = 0;
static uint32_t s_lastMs   = 0;

#define READ_INTERVAL_MS 1000

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== ESP32 – INA237 Courant + Température ===\n");

    // Init I2C
    if (!I2C_Init(SDA_PIN, SCL_PIN)) {
        Serial.println("[MAIN] Echec init I2C");
        return;
    }

    // Init INA237 mesure courant
    if (!INA237_Current_Init()) {
        Serial.println("[MAIN] Echec init INA237 courant");
    }
}

void loop() {
    if (millis() - s_lastMs < READ_INTERVAL_MS) return;
    s_lastMs = millis();

    // ── Mesure courant ────────────────────────────────────
    INA237Measure measure;
    if (INA237_Current_Read(measure)) {
        s_errCount = 0;
        INA237_Current_Print(measure);
    } else {
        s_errCount++;
        Serial.printf("[MAIN] Erreur I2C #%d\n", s_errCount);
        if (s_errCount >= 5) {
            Serial.println("[MAIN] Recovery...");
            I2C_Recover(SDA_PIN, SCL_PIN);
            INA237_Current_Init();
            s_errCount = 0;
        }
        return;
    }

    // ── Mesure température ────────────────────────────────
    float voltage_mV = INA237_ReadVoltageFromTemp();
    if (!isnan(voltage_mV)) {
        Serial.printf("[MAIN] Tension temp : %.2f mV\n", voltage_mV);
    } else {
        Serial.println("[MAIN] Erreur lecture température");
    }
}
