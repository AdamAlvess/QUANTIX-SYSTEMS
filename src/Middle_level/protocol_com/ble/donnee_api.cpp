/*Récupération et envoie des données pour envoie à l’API par bluetooth*/

#include "donnee_api.h"
#include <Arduino.h>

// UUIDs GATT synchronisés avec votre serveur web Flask
#define SERVICE_MONITORING_UUID      "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHAR_CURRENT_UUID            "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHAR_TEMPERATURES_UUID       "c2c95358-0001-447a-9cb8-b0bf0a133401"
#define SERVICE_CONFIG_UUID          "e320d750-2bc4-41d1-8d2b-58d7dc5b49cb"
#define CHAR_ALARM_THRESHOLDS_UUID   "d195e634-1102-4bf5-bc65-02b80a133402"

BLEServer* DonneeApi::pServer = nullptr;
BLECharacteristic* DonneeApi::pCurrentChar = nullptr;
BLECharacteristic* DonneeApi::pTempChar = nullptr;
BLECharacteristic* DonneeApi::pThresholdChar = nullptr;
bool DonneeApi::deviceConnected = false;

void DonneeApi::ServerCallbacks::onConnect(BLEServer* pServer) {
    DonneeApi::deviceConnected = true;
    Serial.println("📱 [BLE] Technicien connecté à la console Quantix !");
}

void DonneeApi::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    DonneeApi::deviceConnected = false;
    Serial.println("🔌 [BLE] Déconnexion du technicien.");
    BLEDevice::startAdvertising(); // Relance l'émission pour le prochain scan
}

void DonneeApi::ThresholdCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 8) { // 2 floats = 8 octets
        float currentMax, tempMax;
        memcpy(&currentMax, value.data(), 4);
        memcpy(&tempMax, value.data() + 4, 4);
        Serial.printf("💾 [BLE] Nouveaux seuils configurés -> I_max: %.2fA, T_max: %.2f°C\n", currentMax, tempMax);
    }
}

void DonneeApi::init(const char* deviceName) {
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Service de télémétrie
    BLEService* pMonitorService = pServer->createService(SERVICE_MONITORING_UUID);
    pCurrentChar = pMonitorService->createCharacteristic(CHAR_CURRENT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pCurrentChar->addDescriptor(new BLE2902());
    pTempChar = pMonitorService->createCharacteristic(CHAR_TEMPERATURES_UUID, BLECharacteristic::PROPERTY_READ);
    pMonitorService->start();

    // Service de configuration
    BLEService* pConfigService = pServer->createService(SERVICE_CONFIG_UUID);
    pThresholdChar = pConfigService->createCharacteristic(CHAR_ALARM_THRESHOLDS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pThresholdChar->setCallbacks(new ThresholdCallbacks());
    pConfigService->start();

    // Diffusion du signal
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_MONITORING_UUID);
    pAdvertising->setScanResponse(true);
    BLEDevice::startAdvertising();
    Serial.println("📡 [BLE] Serveur initialisé dans la couche 'protocol_com' !");
}

void DonneeApi::updateMeasurements(float current, float ntc1, float ntc2, float tempAmb) {
    if (!deviceConnected) return;

    pCurrentChar->setValue((uint8_t*)&current, sizeof(current));
    pCurrentChar->notify();

    float tempBuffer[3] = {ntc1, ntc2, tempAmb};
    pTempChar->setValue((uint8_t*)tempBuffer, sizeof(tempBuffer));
}

bool DonneeApi::isConnected() {
    return deviceConnected;
}