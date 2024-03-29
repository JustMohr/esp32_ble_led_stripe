#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <ArduinoJson.h>
#include <EEPROM.h>

BLEServer* pServer = NULL;
BLECharacteristic* writeColorCharacteristic = NULL;
BLECharacteristic* changeNameCharacteristic = NULL;

#define SERVICE_UUID                   "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_WRITECOLOR "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_CHANGENAME "58479a2e-b3d3-4cbe-8131-1d346e34f349"


int redW = 255;
int greenW = 255;
int blueW = 255;


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("new connection");
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("device disconnected");
    }
};


class WriteColorCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {

      Serial.println("write to color");

      std::string value = pCharacteristic->getValue();

      String data = "";

      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          Serial.print(value[i]);
          data += value[i];
        }
        Serial.println();
        Serial.println("message: "+ data);
        Serial.println("*********");

        //old
        /*JsonDocument doc;
        deserializeJson(doc, data);
        int r = doc["r"];

        redW = doc["r"];
        greenW = doc["g"];
        blueW = doc["b"];*/

        String red = data.substring(0, 3);
        String green = data.substring(4, 7);
        String blue = data.substring(8, 11);

        redW = red.toInt();
        greenW = green.toInt();
        blueW = blue.toInt();

      }
    }
};


class ChangeNameCallbacks: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic *pCharacteristic) {

      Serial.println("write to color");

      std::string value = pCharacteristic->getValue();

      String name = "";

      if (value.length() > 0) {

        for (int i = 0; (i < value.length() && i<25); i++){//-> name max length: 25
          name += value[i];
        }
        Serial.println("new name: "+name);

        BLEDevice::getAdvertising()->stop();
        esp_ble_gap_set_device_name(name.c_str());
        BLEDevice::startAdvertising();

        saveNameToStorage(name);
        
      }

    }


    void saveNameToStorage(String name){
      
      Serial.print("to save: ");
      Serial.println(name);

      int i;
      for(i=0; i<name.length(); i++){
        EEPROM.write(i, name[i]);
      }
      EEPROM.write(i, '\0');
      EEPROM.commit();

      Serial.println("saved");

    }
    
};




void setup() {
  Serial.begin(115200);
  EEPROM.begin(510);
  
  // Create the BLE Device
  String name = readNameFromStorage();
  std::string deviceName = name.c_str();
  BLEDevice::init(deviceName);
  Serial.println("BLE Device initialized");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("BLE Server created");

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  Serial.println("BLE Service created");

  // Create a BLE Characteristic
  writeColorCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_WRITECOLOR,
                      BLECharacteristic::PROPERTY_WRITE
                    );
  changeNameCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_CHANGENAME,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
  );

  writeColorCharacteristic->setCallbacks(new WriteColorCallback());
  changeNameCharacteristic->setCallbacks(new ChangeNameCallbacks());

  String notValue = name.c_str();
  changeNameCharacteristic->setValue((uint8_t*)notValue.c_str(), notValue.length());

  Serial.println("BLE Characteristics created");


  // Start the service
  pService->start();
  Serial.println("BLE Service started");

  //Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true); //true = issue fix
  BLEDevice::startAdvertising();
  Serial.println("Advertising started. Waiting for a client connection to notify...");
  

}

void loop() {

  //int connectedClients = pServer->getConnectedCount();
  //Serial.println("Connected clients: " + String(connectedClients));

  analogWrite(15, redW);
  analogWrite(2, greenW);
  analogWrite(0, blueW);
  delay(2);

}


String readNameFromStorage() {
  String name = "";

  char c = EEPROM.read(0);
  int i =0;
  while(c != '\0' && i<=500){
    name += c;
    i++;
    c = EEPROM.read(i);
  }

  name.trim();
  if(name.length() == 0){
    name = "rgbS";
  }

  Serial.println("device name: " +name);

  return name;

}
