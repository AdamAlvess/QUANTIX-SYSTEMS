#pragma once
#include <Arduino.h>
#include <Wire.h>

// ─────────────────────────────────────────────
//  Pins I2C (ESP32-PICO-D4 – à adapter si besoin)
// ─────────────────────────────────────────────
#define I2C_SDA_PIN     21
#define I2C_SCL_PIN     22
#define I2C_FREQ_HZ     400000   // Fast Mode 400 kHz

// ─────────────────────────────────────────────
//  Timeouts & retries
// ─────────────────────────────────────────────
#define I2C_TIMEOUT_MS  100
#define I2C_MAX_RETRIES 3

// ─────────────────────────────────────────────
//  Codes d'erreur
// ─────────────────────────────────────────────
enum class I2CError : uint8_t {
    OK          = 0,
    DATA_TOO_LONG,
    NACK_ADDR,
    NACK_DATA,
    OTHER,
    TIMEOUT,
    BUS_BUSY
};

class I2CManager {
public:
    static bool     begin();
    static bool     isDevicePresent(uint8_t addr);
    static void     scan(Stream &serial);
    static bool     recovery();

    static I2CError writeReg(uint8_t addr, uint8_t reg,
                             const uint8_t *data, size_t len);
    static I2CError readReg (uint8_t addr, uint8_t reg,
                             uint8_t *data, size_t len);

    static const char* errorStr(I2CError err);

private:
    static I2CError mapWireError(uint8_t code);
};
