#pragma once
#include <Arduino.h>
#include "I2C_Manager.h"

// ─────────────────────────────────────────────
//  Adresses I2C (selon A0/A1 sur le PCB)
//  A1=GND A0=GND → 0x40  (défaut)
//  A1=GND A0=VS  → 0x41
//  A1=VS  A0=GND → 0x44
//  A1=VS  A0=VS  → 0x45
// ─────────────────────────────────────────────
#define INA237_DEFAULT_ADDR  0x40

// ─────────────────────────────────────────────
//  Registres INA237
// ─────────────────────────────────────────────
#define INA237_REG_CONFIG       0x00
#define INA237_REG_ADC_CONFIG   0x01
#define INA237_REG_SHUNT_CAL    0x02
#define INA237_REG_SHUNT_V      0x04
#define INA237_REG_BUS_V        0x05
#define INA237_REG_DIETEMP      0x06
#define INA237_REG_CURRENT      0x07
#define INA237_REG_POWER        0x08
#define INA237_REG_ALERT        0x0B
#define INA237_REG_SOVL         0x0C
#define INA237_REG_SUVL         0x0D
#define INA237_REG_BOVL         0x0E
#define INA237_REG_BUVL         0x0F
#define INA237_REG_TEMP_LIMIT   0x10
#define INA237_REG_MANUFACTURER 0x3E
#define INA237_REG_DEVICE_ID    0x3F

// ─────────────────────────────────────────────
//  Configuration registre 0x00
//  Bits [15:12] : RST, RSTACC, CONVDLY, TEMPCOMP
//  Bits [11:9]  : ADCRANGE (0=±163.84mV, 1=±40.96mV)
// ─────────────────────────────────────────────
#define INA237_CONFIG_RESET     (1 << 15)
#define INA237_ADC_RANGE_HIGH   (0 << 4)  // ±163.84 mV
#define INA237_ADC_RANGE_LOW    (1 << 4)  // ±40.96  mV  (meilleure résolution)

// ─────────────────────────────────────────────
//  ADC Config registre 0x01
//  MODE[3:0] = 0xF → continu courant+tension+temp
//  VBUSCT    = 4   → 1.052 ms
//  VSHCT     = 4   → 1.052 ms
//  VTCT      = 4   → 1.052 ms
//  AVG       = 0   → 1 échantillon
// ─────────────────────────────────────────────
#define INA237_ADC_CONFIG_DEFAULT 0xFB68

// ─────────────────────────────────────────────
//  Valeur de la résistance shunt (Ω)
//  → À adapter selon votre schéma
// ─────────────────────────────────────────────
#define INA237_SHUNT_OHMS       0.01f    // 10 mΩ
#define INA237_MAX_CURRENT_A    10.0f   // Courant max attendu (A)

// Résolution courant LSB = MaxCurrent / 2^15
#define INA237_CURRENT_LSB_A    (INA237_MAX_CURRENT_A / 32768.0f)

// SHUNT_CAL = 13107.2e6 * CURRENT_LSB * R_shunt
#define INA237_SHUNT_CAL_VAL    ((uint16_t)(13107.2e6f * INA237_CURRENT_LSB_A * INA237_SHUNT_OHMS))

struct INA237Data {
    float busVoltage_V;
    float shuntVoltage_mV;
    float current_A;
    float power_W;
    float temperature_C;
    bool  valid;
};

class INA237 {
public:
    explicit INA237(uint8_t addr = INA237_DEFAULT_ADDR);

    bool        begin();
    bool        reset();
    bool        configure();
    bool        verifyId();

    bool        readAll(INA237Data &out);
    float       readBusVoltage();
    float       readShuntVoltage();
    float       readCurrent();
    float       readPower();
    float       readTemperature();

    void        printData(const INA237Data &d, Stream &serial);

private:
    uint8_t     _addr;

    bool        writeReg16(uint8_t reg, uint16_t val);
    bool        readReg16 (uint8_t reg, uint16_t &val);
    int16_t     toSigned16(uint16_t raw);
};
