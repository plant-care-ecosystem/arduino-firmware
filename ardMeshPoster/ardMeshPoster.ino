#include <WiFi.h>
#include <ArduinoJson.h>
#include <cstring>
#include "request.h"

// Poster Configuration
const char* ssid = "Copleyhouse17";
const char* password = "chbos2017";
const char* server = "plantcare-backend.azurewebsites.net";
String userID_temp = "admin";
// Initialize the HTTPS request handler
HttpsHandler httpsHandler;

// Serial communication configuration
#define UART1_TX 5             // Pin 5
#define UART1_RX 4             // Pin 4
HardwareSerial SerialPort1(1); // Start serial port 1

// Function definitions
void wifiConnect();
void parseSerial(void *pvParameters);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Start serial port for receiving data from provisioner
  SerialPort1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
  // Connect to local WiFi network
  wifiConnect();
  // Set the server for the HTTPS handler
  httpsHandler.setServer(server);

  // Test post login
  StaticJsonDocument<200> postDoc;
  postDoc["username"] = "admin";
  postDoc["password"] = "admin";
  String jsonString;
  serializeJson(postDoc, jsonString);
  uint32_t dbId = httpsHandler.requestHandler(jsonString, requestType::POST, requestLocation::LOGIN);
  Serial.println(dbId);

  // FreeRTOS task for handling serial port 1
  xTaskCreate(parseSerial,   // Function to run
              "SerialPort1Task", // Name of the task
              10000,             // Stack size (bytes)
              NULL,              // Parameter to pass
              1,                 // Task priority
              NULL               // Task handle
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}

void wifiConnect() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

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
        postDoc["userId"] = 1;

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
