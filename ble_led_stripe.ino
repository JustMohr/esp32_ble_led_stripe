#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <ArduinoJson.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

int redW = 255;
int greenW = 255;
int blueW = 255;

int redOld = 255;
int greenOld = 255;
int blueOld = 255;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("new connection");
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("device disconnected");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      String json = "";

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          Serial.print(value[i]);
          json += value[i];
        }
        Serial.println();
        Serial.println("message: "+ json);
        Serial.println("*********");

        JsonDocument doc;
        deserializeJson(doc, json);
        int r = doc["r"];

        redW = doc["r"];
        greenW = doc["g"];
        blueW = doc["b"];

      }
    }
};

void setup() {
  Serial.begin(115200);
  
  // Create the BLE Device
  BLEDevice::init("Esp32");
  Serial.println("BLE Device initialized");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("BLE Server created");

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  Serial.println("BLE Service created");

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic->setCallbacks(new MyCallbacks());
  Serial.println("BLE Characteristic created");

  // Start the service
  pService->start();
  Serial.println("BLE Service started");

  //Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  BLEDevice::startAdvertising();
  Serial.println("Advertising started. Waiting for a client connection to notify...");
  

}

int counter = 0;
void loop() {

  int connectedClients = pServer->getConnectedCount();
  //Serial.println("Connected clients: " + String(connectedClients));
  

  bool write = false;
  if(redOld != redW || greenOld != greenW || blueOld != blueW){
      redOld = redW;
      greenOld = greenW;
      blueOld = blueW;
      counter = 0;
  }

  if(counter == 50){
    write = true;
  }



  if(connectedClients >= 1 && write == true){
    
      String responsJson = "{\"r\":" + String(redW) + ",\"g\":" + String(greenW) + ",\"b\":" + String(blueW) + "}";
      String notValue = responsJson.c_str();
      pCharacteristic->setValue((uint8_t*)notValue.c_str(), notValue.length());
      pCharacteristic->notify();
      Serial.println("Notified value: " + String(notValue));

  }


  analogWrite(15, redW);
  analogWrite(2, greenW);
  analogWrite(0, blueW);
  delay(2);
  counter ++;

}
