#pragma once
#include <Arduino.h>
#include <Wire.h>

// ─────────────────────────────────────────────
//  Adresse & Registres INA237
// ─────────────────────────────────────────────
#define INA237_ADDR             0x40

#define INA237_REG_CONFIG       0x00
#define INA237_REG_ADC_CONFIG   0x01
#define INA237_REG_SHUNT_CAL    0x02
#define INA237_REG_SHUNT_V      0x04
#define INA237_REG_BUS_V        0x05
#define INA237_REG_CURRENT      0x07

// ─────────────────────────────────────────────
//  Résistance shunt & courant max
//  → À adapter selon votre PCB
// ─────────────────────────────────────────────
#define INA237_SHUNT_OHMS       0.005f    // 5 mΩ
#define INA237_MAX_CURRENT_A    3.3f    // Courant max attendu

// LSB courant = MaxCurrent / 2^15
#define INA237_CURRENT_LSB      (INA237_MAX_CURRENT_A / 32768.0f)

// SHUNT_CAL = 13107.2e6 * LSB * R_shunt
#define INA237_SHUNT_CAL_VAL    ((uint16_t)(13107.2e6f * INA237_CURRENT_LSB * INA237_SHUNT_OHMS))

// ─────────────────────────────────────────────
//  Structure de mesure
// ─────────────────────────────────────────────
struct INA237Measure {
    float current_A;        // Courant mesuré (A)
    float busVoltage_V;     // Tension bus (V)
    float shuntVoltage_mV;  // Tension shunt (mV)
    float power_W;          // Puissance calculée (W)
    bool  valid;
};

bool            INA237_Current_Init();
bool            INA237_Current_Read(INA237Measure &out);
void            INA237_Current_Print(const INA237Measure &m);
