#include <Arduino.h>
#include "Middle_level/temperature/INA237/I2C_Init.h"
#include "Middle_level/temperature/INA237/INA237_Temp.h"
#include "Middle_level/temperature/INA237/I2C_Error.h"

static uint8_t  s_errCount = 0;
static uint32_t s_lastMs   = 0;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== ESP32 – INA237 I2C ===\n");

    if (!I2C_Init(SDA_PIN, SCL_PIN)) {
        Serial.println("[MAIN] Echec init – vérifier le câblage");
    }
}

void loop() {
    if (millis() - s_lastMs < 1000) return;
    s_lastMs = millis();

    float temp = INA237_ReadTemperature();

    if (isnan(temp)) {
        s_errCount++;
        Serial.printf("[MAIN] Erreur #%d\n", s_errCount);

        if (s_errCount >= 5) {
            Serial.println("[MAIN] Trop d'erreurs → recovery");
            I2C_Recover(SDA_PIN, SCL_PIN);
            s_errCount = 0;
        }
    } else {
        s_errCount = 0;
    }
}
