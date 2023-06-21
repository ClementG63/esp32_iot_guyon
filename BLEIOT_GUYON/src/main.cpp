#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t value = 0;

// UUID du service et de la caractéristique
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(9600);

  // Créer le périphérique BLE
  BLEDevice::init("ESP32_BLE");

  // Créer le serveur BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Créer le service BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Créer la caractéristique BLE
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ   |
                                         BLECharacteristic::PROPERTY_WRITE  |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  
  pCharacteristic->addDescriptor(new BLE2902());

  // Démarrer le service
  pService->start();

  // Démarrer l'annonce
  pServer->getAdvertising()->start();
  Serial.println("L'ESP32 BLE est prêt à accepter une connexion.");
}

void loop() {
  if (deviceConnected) {
    pCharacteristic->setValue(&value, 1);
    pCharacteristic->notify();
    value++;
    delay(10); // Envoi de la notification toutes les 10ms
  }
  // Si le périphérique n'est plus connecté, redémarrer l'annonce
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // Attendre un peu avant de redémarrer l'annonce
    pServer->startAdvertising();
    Serial.println("L'ESP32 BLE est prêt à accepter une nouvelle connexion.");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}
