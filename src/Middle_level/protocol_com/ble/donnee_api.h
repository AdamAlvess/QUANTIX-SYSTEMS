/*Récupération et envoie des données pour envoie à l’API par bluetooth*/

#ifndef DONNEE_API_H
#define DONNEE_API_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class DonneeApi {
private:
    static BLEServer* pServer;
    static BLECharacteristic* pCurrentChar;
    static BLECharacteristic* pTempChar;
    static BLECharacteristic* pThresholdChar;
    static bool deviceConnected;

    class ServerCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    };
    
    class ThresholdCallbacks : public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic* pCharacteristic) override;
    };

public:
    static void init(const char* deviceName);
    static void updateMeasurements(float current, float ntc1, float ntc2, float tempAmb);
    static bool isConnected();
};

#endif