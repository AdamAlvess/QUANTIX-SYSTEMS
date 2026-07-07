#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────
//  Pins MikroBus sur ESP32-PICO-D4
//  Selon pinout standard MikroBus :
//  AN  → GPIO34 (entrée analogique)
//  RST → GPIO5
//  CS  → GPIO15
//  SCK → GPIO18  (SPI)
//  MISO→ GPIO19  (SPI)
//  MOSI→ GPIO23  (SPI)
//  PWM → GPIO25
//  INT → GPIO35  (entrée interruption)
//  RX  → GPIO16  (UART)
//  TX  → GPIO17  (UART)
//  SCL → GPIO22  (I2C)
//  SDA → GPIO21  (I2C)
// ─────────────────────────────────────────────
#define MIKROBUS_AN_PIN     34
#define MIKROBUS_RST_PIN    5
#define MIKROBUS_CS_PIN     15
#define MIKROBUS_SCK_PIN    18
#define MIKROBUS_MISO_PIN   19
#define MIKROBUS_MOSI_PIN   23
#define MIKROBUS_PWM_PIN    25
#define MIKROBUS_INT_PIN    35
#define MIKROBUS_RX_PIN     16
#define MIKROBUS_TX_PIN     17
#define MIKROBUS_SCL_PIN    22
#define MIKROBUS_SDA_PIN    21

// ─────────────────────────────────────────────
//  Type de module branché sur le MikroBus
// ─────────────────────────────────────────────
enum class MikroBusModule : uint8_t {
    LORA_RN2483,    // UART
    NONE
};

bool MikroBus_InitGPIO(MikroBusModule module);
void MikroBus_Reset();
void MikroBus_PrintConfig();
