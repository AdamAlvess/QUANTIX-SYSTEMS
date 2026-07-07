#pragma once
#include <Arduino.h>
#include <Wire.h>

#define INA237_ADDR     0x40
#define INA237_REG_CONFIG 0x00

bool I2C_Init(uint8_t sda, uint8_t scl, uint32_t freq = 400000);
