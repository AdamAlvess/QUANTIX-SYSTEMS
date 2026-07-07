#include "MikroBus_LoRa.h"

// UART dédié au RN2483
static HardwareSerial LoRaSerial(LORA_UART_NUM);

// ─────────────────────────────────────────────
//  Envoi d'une commande AT et lecture réponse
// ─────────────────────────────────────────────
static String sendCommand(const String &cmd, uint32_t timeout = LORA_CMD_TIMEOUT_MS) {
    LoRaSerial.println(cmd);
    Serial.printf("[LoRa] >> %s\n", cmd.c_str());

    uint32_t start = millis();
    String response = "";

    while (millis() - start < timeout) {
        if (LoRaSerial.available()) {
            char c = LoRaSerial.read();
            response += c;
            if (response.endsWith("\r\n")) break;
        }
    }

    response.trim();
    Serial.printf("[LoRa] << %s\n", response.c_str());
    return response;
}

// ═══════════════════════════════════════════════════════════
//  BLOC 1 — Initialisation UART + Reset RN2483
// ═══════════════════════════════════════════════════════════
bool LoRa_Init() {
    Serial.println("[LoRa] Initialisation MikroBus RN2483...");

    // Démarrage UART
    LoRaSerial.begin(LORA_UART_BAUD, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
    delay(100);

    if (!LoRaSerial) {
        Serial.println("[LoRa] Erreur démarrage UART");
        return false;
    }

    // Reset matériel via pin RST (actif bas)
    pinMode(LORA_RST_PIN, OUTPUT);
    digitalWrite(LORA_RST_PIN, LOW);
    delay(100);
    digitalWrite(LORA_RST_PIN, HIGH);
    delay(500);  // Attente boot RN2483

    // Vérification réponse boot ("RN2483 X.X.X ...")
    String boot = "";
    uint32_t start = millis();
    while (millis() - start < 2000) {
        if (LoRaSerial.available()) {
            boot += (char)LoRaSerial.read();
            if (boot.endsWith("\r\n")) break;
        }
    }
    boot.trim();
    Serial.printf("[LoRa] Boot : %s\n", boot.c_str());

    if (!boot.startsWith("RN2483")) {
        Serial.println("[LoRa] Module RN2483 non détecté");
        return false;
    }

    // Réinitialisation MAC
    sendCommand("mac reset 868");

    Serial.println("[LoRa] Init OK");
    return true;
}

// ═══════════════════════════════════════════════════════════
//  BLOC 2 — Connexion LoRaWAN OTAA
// ═══════════════════════════════════════════════════════════
LoRaStatus LoRa_Join() {
    Serial.println("[LoRa] Tentative de join OTAA...");

    // Configuration des credentials
    String r;
    r = sendCommand("mac set deveui "  + String(LORA_DEV_EUI));
    if (r != "ok") return LoRaStatus::ERR_JOIN;

    r = sendCommand("mac set appeui "  + String(LORA_APP_EUI));
    if (r != "ok") return LoRaStatus::ERR_JOIN;

    r = sendCommand("mac set appkey "  + String(LORA_APP_KEY));
    if (r != "ok") return LoRaStatus::ERR_JOIN;

    // Paramètres réseau
    sendCommand("mac set adr on");
    sendCommand("mac set dr 0");        // DR0 = portée maximale

    // Lancement du join
    r = sendCommand("mac join otaa", 10000);
    if (r != "accepted") {
        Serial.printf("[LoRa] Join refusé : %s\n", r.c_str());
        return LoRaStatus::ERR_JOIN;
    }

    Serial.println("[LoRa] Join OTAA réussi !");
    return LoRaStatus::OK;
}

// ═══════════════════════════════════════════════════════════
//  BLOC 3 — Envoi des données (tension + température)
//  Payload : 4 octets
//    [0-1] tension_mV × 100 (int16)
//    [2-3] temp_C    × 100 (int16)
// ═══════════════════════════════════════════════════════════
LoRaStatus LoRa_SendData(float voltage_mV, float temp_C) {

    // Encodage en hex (big-endian)
    int16_t v = (int16_t)(voltage_mV * 100.0f);
    int16_t t = (int16_t)(temp_C    * 100.0f);

    char payload[9];
    snprintf(payload, sizeof(payload), "%04X%04X",
             (uint16_t)v, (uint16_t)t);

    Serial.printf("[LoRa] Payload : %s  (V=%.2fmV T=%.2f°C)\n",
                  payload, voltage_mV, temp_C);

    // Envoi sur le port 1, non confirmé
    String cmd = "mac tx uncnf 1 " + String(payload);
    String r   = sendCommand(cmd, 5000);

    if (r == "mac_tx_ok") {
        Serial.println("[LoRa] Envoi OK");
        return LoRaStatus::OK;
    }

    if (r.startsWith("mac_rx")) {
        Serial.printf("[LoRa] Envoi OK + downlink reçu : %s\n", r.c_str());
        return LoRaStatus::OK;
    }

    Serial.printf("[LoRa] Erreur envoi : %s\n", r.c_str());
    return LoRaStatus::ERR_SEND;
}

// ─────────────────────────────────────────────
//  Messages d'erreur lisibles
// ─────────────────────────────────────────────
const char* LoRa_GetStatusStr(LoRaStatus s) {
    switch (s) {
        case LoRaStatus::OK:          return "OK";
        case LoRaStatus::ERR_UART:    return "Erreur UART";
        case LoRaStatus::ERR_RESET:   return "Erreur reset module";
        case LoRaStatus::ERR_JOIN:    return "Join LoRaWAN échoué";
        case LoRaStatus::ERR_SEND:    return "Envoi données échoué";
        case LoRaStatus::ERR_TIMEOUT: return "Timeout";
        default:                      return "Erreur inconnue";
    }
}
