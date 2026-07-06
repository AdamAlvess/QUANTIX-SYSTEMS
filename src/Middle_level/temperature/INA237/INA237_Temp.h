#pragma once
#include <Arduino.h>
#include <Wire.h>

#define INA237_ADDR        0x40
#define INA237_REG_DIETEMP 0x06

// Conversion température → tension (mV)
// Formule : V(mV) = Temp(°C) * 10.0
// Ex : 25°C → 250 mV
#define TEMP_TO_VOLTAGE_FACTOR 10.0f

float INA237_ReadVoltageFromTemp();
