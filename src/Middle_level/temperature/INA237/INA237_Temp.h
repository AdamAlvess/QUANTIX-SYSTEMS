#pragma once
#include <Arduino.h>
#include <Wire.h>

#define INA237_ADDR        0x40
#define INA237_REG_DIETEMP 0x06

// Conversion température → tension (mV)
// Formule : V(mV) = Temp(°C) * 10.0
// Ex : 25°C → 250 mV
#define TEMP_MIN_C      -40.0f
#define TEMP_MAX_C      125.0f
#define VOLTAGE_MAX_MV   40.96f

#define TEMP_TO_VOLTAGE_FACTOR  (VOLTAGE_MAX_MV / TEMP_MAX_C)  // = 0.32768 mV/°C

float INA237_ReadVoltageFromTemp();
