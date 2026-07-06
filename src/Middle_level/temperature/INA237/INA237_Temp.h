#pragma once
#include <Arduino.h>
#include <Wire.h>

#define INA237_ADDR        0x40
#define INA237_REG_DIETEMP 0x06

float INA237_ReadTemperature();
