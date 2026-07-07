#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

// ─────────────────────────────────────────────
//  Pins MikroBus sur ESP32-PICO-D4
//  TX → GPIO17, RX → GPIO16, RST → GPIO5
// ─────────────────────────────────────────────
#define LORA_UART_NUM       2
#define LORA_TX_PIN         17
#define LORA_RX_PIN         16
#define LORA_RST_PIN        5
#define LORA_UART_BAUD      57600
#define LORA_CMD_TIMEOUT_MS 2000

// ─────────────────────────────────────────────
//  Credentials LoRaWAN OTAA
//  → À remplacer par vos valeurs TTN/Chirpstack
// ─────────────────────────────────────────────
#define LORA_DEV_EUI   "0000000000000000"
#define LORA_APP_EUI   "0000000000000000"
#define LORA_APP_KEY   "00000000000000000000000000000000"

// ─────────────────────────────────────────────
//  Codes d'état
// ─────────────────────────────────────────────
enum class LoRaStatus : uint8_t {
    OK = 0,
    ERR_UART,
    ERR_RESET,
    ERR_JOIN,
    ERR_SEND,
    ERR_TIMEOUT
};

bool        LoRa_Init();
LoRaStatus  LoRa_Join();
LoRaStatus  LoRa_SendData(float voltage_mV, float temp_C);
const char* LoRa_GetStatusStr(LoRaStatus s);
