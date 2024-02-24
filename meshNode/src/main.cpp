#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h> // Include the DHT library
// Include namedMesh.h from ./lib
#include "namedMesh.h"

// Mesh configuration
Scheduler userScheduler; // to control mesh tasks
#define   MESH_SSID       "plantware-mesh"
#define   MESH_PASSWORD   "hest-67890"
#define   MESH_PORT       5555
namedMesh  mesh;
String nodeName;

// DHT11 sensor configuration
#define DHTPIN 20 // Define the pin for DHT11 data
#define DHTTYPE DHT11 // Define the type of DHT sensor
DHT dht(DHTPIN, DHTTYPE); // Create a DHT object

// Function declarations
String createJsonString(float temperature_f, float temperature_c, float humidity);
void dhtReadTask(void *pvParameters);
void sendData(void *pvParameters);

// Setup
void setup() {
  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Setup the mesh network
  mesh.setDebugMsgTypes(ERROR);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  // Set mesh node name to the node id
  nodeName = String(mesh.getNodeId());
  mesh.setName(nodeName); // This needs to be an unique name! 

  // Handle message receiving
  mesh.onReceive([](uint32_t from, String &msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  });
  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  // Handle changed connections
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });

  // Start the DHT sensor
  dht.begin();

  // // Create FreeRTOS task for reading from DT11
  // xTaskCreate(dhtReadTask,      // Task function
  //             "DHTReadTask",    // Task name
  //             10000,             // Stack size
  //             NULL,              // Task parameters
  //             2,                 // Task priority
  //             NULL);             // Task handle

  // Create FreeRTOS task for message sending
  xTaskCreate(sendData,      // Task function
              "SendDataTask",    // Task name
              10000,             // Stack size
              NULL,              // Task parameters
              1,                 // Task priority
              NULL);             // Task handle

}

// Loop (most code should be in tasks)
void loop() {
  mesh.update();
}

// Task function to read DHT sensor data
void dhtReadTask(void *pvParameters) {
  (void)pvParameters; // Unused parameter

  for (;;) {
    // Read sensor values
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Print the values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

    // Wait for 2 seconds before the next reading
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// Task function to send data over mesh network
void sendData(void *pvParameters) {
  (void)pvParameters; // Unused parameter

  for (;;) {
    // Read sensor values
    String msg = createJsonString(dht.readTemperature(true), dht.readTemperature(), dht.readHumidity());
    // String msg = "Humidity: " + String(dht.readHumidity()) + "%, Temperature: " + String(dht.readTemperature()) + "°C" + " from " + nodeName;
    String to = "prov";
    // Send data over mesh network
    mesh.sendSingle(to, msg);

    // Wait for 5 seconds before the next message
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

// Put sensor data into a JSON string
String createJsonString(float temperature_f, float temperature_c, float humidity) {
  StaticJsonDocument<200> doc;

  doc["nodeId"] = nodeName;
  doc["temperatureF"] = temperature_f;
  doc["temperatureC"] = temperature_c;
  doc["humidity"] = humidity;

  String jsonString;
  serializeJson(doc, jsonString);

  return jsonString;
}