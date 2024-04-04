#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <cstring>
#include "request.h"

// EEPROM configuration
#define BLE_STORED 0
#define SSID 8
#define PASSWORD 136
#define USER_ID 264
#define EEPROM_UNIT_SIZE 128

// BLE configuration
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

// Poster Configuration
// const char* ssid = "CS-ROGFLOWX13";
// const char* password = "58^s324D";
String ssid;
String password;
const char* server = "plantcare-backend.azurewebsites.net";
String userID;
int runningFlag = 0;
// Initialize the HTTPS request handler
HttpsHandler httpsHandler;

// Serial communication configuration
#define UART1_TX 5             // Pin 5
#define UART1_RX 4             // Pin 4
HardwareSerial SerialPort1(1); // Start serial port 1

// Function definitions
void wifiConnect();
void parseSerial(void *pvParameters);

// Loop through EEPROM and read each byte into string
String readEEPROMChars(int offset) {
    Serial.printf("Reading EEPROM at offset: %d\n", offset);
    String id = "";
    for(int i = 0; i < EEPROM_UNIT_SIZE; i++) {
        // Serial.printf("Reading EEPROM at offset: %d\n", offset + i);
        char c = EEPROM.read(offset + i);
        // Serial.printf("Char: %c\n", c);
        // break if null terminator is found
        if(c == '\0') {
            break;
        }
        id += c;
    }
    Serial.printf("EEPROM read returned: %s\n", id);
    Serial.println();
    return id;

    //   String id;

    // if(isTypeName) {
    //     char buffer[EEPROM_NAME_SIZE];
    //     EEPROM.get(offset, buffer);
    //     id = String(buffer);
    // }
    // else {
    //     char buffer[EEPROM_UNIT_SIZE];
    //     EEPROM.get(offset, buffer);
    //     id = String(buffer);
    // }

    // return id.toInt();

}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String value = pCharacteristic->getValue();
      
      // Json parse the value
      StaticJsonDocument<200> doc;
      deserializeJson(doc, value);

      String ssidt = doc["wifiSSID"];
      ssid = ssidt;
      String passwordt = doc["wifiPassword"];
      password = passwordt;
      String userIDt = doc["userId"];
      userID = userIDt;


      // Save all values to EEPROM
      EEPROM.put(BLE_STORED, 1);
      EEPROM.commit();
      EEPROM.put(SSID, ssid);
      EEPROM.commit();
      EEPROM.put(PASSWORD, password);
      EEPROM.commit();
      EEPROM.put(USER_ID, userID);
      EEPROM.commit();

      // Connect to WiFi
      wifiConnect();

      if(runningFlag == 0) {
        // Set the server for the HTTPS handler
        httpsHandler.setServer(server);

        // FreeRTOS task for handling serial port 1
        xTaskCreate(parseSerial,   // Function to run
                    "SerialPort1Task", // Name of the task
                    10000,             // Stack size (bytes)
                    NULL,              // Parameter to pass
                    1,                 // Task priority
                    NULL               // Task handle
        );
      }
    }
};

MyCallbacks myCallbacks;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Start serial port for receiving data from provisioner
  SerialPort1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  // Initialize EEPROM
  EEPROM.begin(512);

  Serial.println(WiFi.macAddress());
  // Connect to local WiFi network
   Serial.println("Starting BLE work!");

  BLEDevice::init("PlantWare-HUB-1");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(&myCallbacks);

  // pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  // Check if BLE_STORED is true
  int bleStored = 0;
  bleStored = EEPROM.read(BLE_STORED);
  Serial.printf("BLE stored: %d\n", bleStored);
  if(bleStored == 1) {
    ssid = readEEPROMChars(SSID);
    password = readEEPROMChars(PASSWORD);
    userID = readEEPROMChars(USER_ID);
    wifiConnect();

    // Set the server for the HTTPS handler
    httpsHandler.setServer(server);

    // FreeRTOS task for handling serial port 1
    xTaskCreate(parseSerial,   // Function to run
                "SerialPort1Task", // Name of the task
                10000,             // Stack size (bytes)
                NULL,              // Parameter to pass
                1,                 // Task priority
                NULL               // Task handle
    );
  }
  else {
    Serial.println("Waiting for BLE configuration...");
  }
  

  // // Test post login
  // StaticJsonDocument<200> postDoc;
  // postDoc["username"] = "admin";
  // postDoc["password"] = "admin";
  // String jsonString;
  // serializeJson(postDoc, jsonString);
  // uint32_t dbId = httpsHandler.requestHandler(jsonString, requestType::POST, requestLocation::LOGIN);
  // Serial.println(dbId);

  
}

void loop() {
  // put your main code here, to run repeatedly:

}

void wifiConnect() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), password.c_str());

  // Attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
}


// Function to parse serial data
void parseSerial(void *pvParameters) {
  runningFlag = 1;
  for (;;) {
    if (SerialPort1.available()) {
      String input = SerialPort1.readStringUntil('\n');
      Serial.println(input);
      StaticJsonDocument<200> doc;
      deserializeJson(doc, input);
      String type = doc["type"];

      // Check for message type
      if(type == "config") {
        // Create JSON doc for sending a new plant to the backend
        Serial.println("Config message received");
        StaticJsonDocument<200> postDoc;
        postDoc["name"] = doc["name"];
        postDoc["userId"] = userID;

        String jsonString;
        serializeJson(postDoc, jsonString);
      int32_t dbId = -1;
      while ((dbId = httpsHandler.requestHandler(jsonString, requestType::PUT, requestLocation::PLANT)) == -1) {
          // You can add a delay here if you want to avoid flooding the server with requests
          delay(5000);
      }

        StaticJsonDocument<200> returnDoc;
        returnDoc["type"] = "plantadd";
        returnDoc["nodeId"] = doc["nodeId"];
        returnDoc["dbId"] = dbId;

        serializeJson(returnDoc, jsonString);

        SerialPort1.println(jsonString);

      }
      else if(type == "sensorhello") {
        // Create JSON doc for sending a new sensor to the backend
        Serial.println("Sensor config message received");
        StaticJsonDocument<200> postDoc;
        postDoc["name"] = doc["name"];
        uint32_t plant_id = doc["plantId"];
        postDoc["plantId"] = plant_id;

        String jsonString;
        serializeJson(postDoc, jsonString);
        // int32_t dbId = httpsHandler.requestHandler(jsonString, requestType::PUT, requestLocation::SENSOR);
        int32_t dbId = -1;
        while ((dbId = httpsHandler.requestHandler(jsonString, requestType::PUT, requestLocation::SENSOR)) == -1) {
            // You can add a delay here if you want to avoid flooding the server with requests
            delay(5000);
        }

        Serial.printf("Sensor ID: %u", dbId);

        StaticJsonDocument<200> returnDoc;
        returnDoc["type"] = "sensoradd";
        returnDoc["nodeId"] = doc["nodeId"];
        returnDoc["sensorNumber"] = doc["sensorNumber"];
        returnDoc["dbId"] = dbId;

        serializeJson(returnDoc, jsonString);

        SerialPort1.println(jsonString);
      }
      else if(type == "data") {
        Serial.println("Sensor data message received");
        StaticJsonDocument<200> postDoc;
        postDoc["sensorId"] = doc["sensorId"];
        postDoc["data"] = doc["data"];

        String jsonString;
        serializeJson(postDoc, jsonString);
        // int32_t dbId = httpsHandler.requestHandler(jsonString, requestType::POST, requestLocation::DATA);
        int32_t dbId = -1;
        while ((dbId = httpsHandler.requestHandler(jsonString, requestType::PUT, requestLocation::DATA)) == -1) {
            // You can add a delay here if you want to avoid flooding the server with requests
            delay(5000);
        }

        StaticJsonDocument<200> returnDoc;
        returnDoc["type"] = "sensoradd";
        returnDoc["nodeId"] = doc["nodeId"];
        returnDoc["dbId"] = dbId;

        serializeJson(returnDoc, jsonString);

        // SerialPort1.println(jsonString);

      }
    }
  }
}
