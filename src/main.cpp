#include <Arduino.h>
#include "Middle_level/temperature/INA237/I2C_Init.h"
#include "Middle_level/temperature/INA237/INA237_Temp.h"
#include "Middle_level/temperature/INA237/I2C_Error.h"
#include "Middle_level/gpio/MikroBus.h"

static uint8_t  s_errCount  = 0;
static uint32_t s_lastMs    = 0;
static bool     s_loraReady = false;

// Envoi LoRa toutes les 30 secondes
#define LORA_SEND_INTERVAL_MS 30000
static uint32_t s_lastLoraMs = 0;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n=== ESP32 – INA237 + LoRa MikroBus ===\n");

    // Init I2C
    if (!I2C_Init(SDA_PIN, SCL_PIN)) {
        Serial.println("[MAIN] Echec init I2C");
    }

    // Init LoRa
    if (!LoRa_Init()) {
        Serial.println("[MAIN] Module LoRa non disponible");
    } else {
        LoRaStatus join = LoRa_Join();
        if (join == LoRaStatus::OK) {
            s_loraReady = true;
        } else {
            Serial.printf("[MAIN] LoRa join échoué : %s\n",
                          LoRa_GetStatusStr(join));
        }
    }
}

void loop() {
    uint32_t now = millis();

    // Lecture INA237 toutes les secondes
    if (now - s_lastMs >= 1000) {
        s_lastMs = now;

        float voltage_mV = INA237_ReadVoltageFromTemp();

        if (isnan(voltage_mV)) {
            s_errCount++;
            Serial.printf("[MAIN] Erreur I2C #%d\n", s_errCount);
            if (s_errCount >= 5) {
                I2C_Recover(SDA_PIN, SCL_PIN);
                s_errCount = 0;
            }
        } else {
            s_errCount = 0;
        }
    }

    // Envoi LoRa toutes les 30 secondes
    if (s_loraReady && (now - s_lastLoraMs >= LORA_SEND_INTERVAL_MS)) {
        s_lastLoraMs = now;

        float voltage_mV = INA237_ReadVoltageFromTemp();

        if (!isnan(voltage_mV)) {
            // Reconversion tension → température pour l'envoi
            float temp_C = voltage_mV / (40.96f / 125.0f);

            LoRaStatus st = LoRa_SendData(voltage_mV, temp_C);
            if (st != LoRaStatus::OK) {
                Serial.printf("[MAIN] Erreur LoRa : %s\n",
                              LoRa_GetStatusStr(st));
            }
        }
    }
}
