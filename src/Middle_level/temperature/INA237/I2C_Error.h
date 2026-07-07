#pragma once
#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 21
#define SCL_PIN 22

enum class I2CStatus : uint8_t {
    OK        = 0,
    NACK_ADDR = 2,
    NACK_DATA = 3,
    UNKNOWN   = 4,
    TIMEOUT   = 5
};

const char* I2C_GetErrorStr(uint8_t wireCode);
bool        I2C_Recover(uint8_t sda, uint8_t scl);
