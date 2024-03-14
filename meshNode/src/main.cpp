#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cstring>
// #include <EEPROM.h>
// #include <Adafruit_Sensor.h>
// #include <DHT.h> // Include the DHT library
// Include namedMesh.h from ./lib
#include "sensor.h"
#include "namedMesh.h"

// Mesh configuration
Scheduler userScheduler; // to control mesh tasks
#define   MESH_SSID       "plantware-mesh"
#define   MESH_PASSWORD   "hest-67890"
#define   MESH_PORT       5555
namedMesh  mesh;
String nodeName;
String to = "prov";

// DHT11 sensor configuration
// #define DHTPIN 20 // Define the pin for DHT11 data
// #define DHTTYPE DHT11 // Define the type of DHT sensor
#define SENSOR_1_PIN 20
#define SENSOR_1_TYPE MESH_DHT11
Sensor* sensor1;

// EEPROM configuration
#define EEPROM_SIZE 512
#define NAME_CHANGE_FLAG 0
#define NODE_NAME 1
char name_buffer[512];
uint32_t ncf = 0;

// Send message types
enum SndMessageType {
  HELLO,
  NAME,
  DATA
};



// Function declarations
void meshInit();
String createJsonString(float temperature_f, float temperature_c, float humidity);
void dhtReadTask(void *pvParameters);
void sendData(void *pvParameters);


// Setup
void setup() {
  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Initialize EEPROM
  // EEPROM.begin(EEPROM_SIZE);

  // Initialize the mesh network
  meshInit();

  // // Create new sensor factory
  // SensorFactory* sensorFactory = new DHTSensorFactory();
  // sensor1 = sensorFactory->createSensor(SENSOR_1_TYPE, SENSOR_1_PIN);

  // // Create FreeRTOS task for reading from DT11
  // xTaskCreate(dhtReadTask,      // Task function
  //             "DHTReadTask",    // Task name
  //             10000,             // Stack size
  //             NULL,              // Task parameters
  //             2,                 // Task priority
  //             NULL);             // Task handle

  // Create FreeRTOS task for message sending
  // xTaskCreate(sendData,      // Task function
  //             "SendDataTask",    // Task name
  //             10000,             // Stack size
  //             NULL,              // Task parameters
  //             1,                 // Task priority
  //             NULL);             // Task handle

}

// Loop (most code should be in tasks)
void loop() {
  mesh.update();
}

void sendMessageHandler(String msg, SndMessageType type) {
  switch (type) {
    case HELLO: 
    {
      // Send message containing node ID and current Name
      StaticJsonDocument<200> doc;
      doc["type"] = "hello";
      doc["nodeId"] = mesh.getNodeId();

      // // Check if there is a name already stored in EEPROM
      // if(EEPROM.read(NAME_CHANGE_FLAG) == 1) {
      //   char name[EEPROM_SIZE - NODE_NAME];
      //   for(int i = 0; i < EEPROM_SIZE - NODE_NAME; i++) {
      //     name[i] = EEPROM.read(NODE_NAME + i);
      //     // break if null terminator is found
      //     if(name[i] == '\0') {
      //       break;
      //     }
      //   }
      //   doc["name"] = name;
      // } 


      if(ncf == 1) {
        doc["name"] = name_buffer;
      }
      // If no name is stored in EEPROM, set name to node ID
      else {
        doc["name"] = mesh.getName();
      }

      // Send hello message
      String jsonString;
      serializeJson(doc, jsonString);
      Serial.println("Sending hello message");
      mesh.sendSingle(to, jsonString);
      
      break;
    }
    case NAME:
    {
      
      break;
    }
    case DATA:
      mesh.sendSingle(to, msg);
      break;
    default:
      break;
  }
}

void receiveMessageHandler(String msg) {
  // Deserialize the message
  StaticJsonDocument<200> doc;
  deserializeJson(doc, msg);

  // Check the type of message
  const char* type = doc["type"];
  if(strcmp(type, "config") == 0) {
    // Store the name in EEPROM
    Serial.println("Received config message");
    String name = doc["name"];
    // for(int i = 0; i < name.length(); i++) {
    //   EEPROM.write(NODE_NAME + i, name[i]);
    // }
    // Add null terminator
    // EEPROM.write(NODE_NAME + name.length(), '\0');
    // // Set name change flag to 1
    // EEPROM.write(NAME_CHANGE_FLAG, 1);
    // EEPROM.commit();

    // Store the name into the name buffer
    // inputting a null terminator when name is empty
    if(name.length() == 0) {
      name_buffer[0] = '\0';
      ncf = 0;
    }
    else {
      for(int i = 0; i < name.length(); i++) {
        name_buffer[i] = name[i];
      }
      name_buffer[name.length()] = '\0';
      ncf = 1;
    }
    

    // Send another hello message to update the name
    sendMessageHandler("", HELLO);
  }
  // return;
}

void meshInit() {
  // Setup the mesh network
  mesh.setDebugMsgTypes(ERROR);  // set before init() so that you can see startup messages
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  // Set mesh node name to the node id
  nodeName = String(mesh.getNodeId());
  mesh.setName(nodeName); // This needs to be an unique name! 

  // Handle message receiving
  // mesh.onReceive([](uint32_t from, String &msg) {
  //   Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  // });
  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
    receiveMessageHandler(msg);
  });

  // Handle changed connections
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    // Send hello type message
    sendMessageHandler("", HELLO);
  });
}

// // Task function to read DHT sensor data
// void dhtReadTask(void *pvParameters) {
//   (void)pvParameters; // Unused parameter

//   for (;;) {
//     // Read sensor values
//     float humidity = dht.readHumidity();
//     float temperature = dht.readTemperature();

//     // Print the values to serial monitor
//     Serial.print("Humidity: ");
//     Serial.print(humidity);
//     Serial.print("%, Temperature: ");
//     Serial.print(temperature);
//     Serial.println("°C");

//     // Wait for 2 seconds before the next reading
//     vTaskDelay(pdMS_TO_TICKS(2000));
//   }
// }

// Task function to send data over mesh network
void sendData(void *pvParameters) {
  (void)pvParameters; // Unused parameter

  for (;;) {
    // Read sensor values
    sensor1->readSensor();

    // Print temperature and humidity to serial monitor
    // Serial.printf("Humidity: %f\n", sensor1->getHumidity());
    // Serial.printf("Temperature C: %f\n", sensor1->getTemperatureC());
    // Serial.printf("Temperature F: %f\n", sensor1->getTemperatureF());

    // // Read sensor values
    // String msg = createJsonString(dht.readTemperature(true), dht.readTemperature(), dht.readHumidity());
    // // String msg = "Humidity: " + String(dht.readHumidity()) + "%, Temperature: " + String(dht.readTemperature()) + "°C" + " from " + nodeName;
    // String to = "prov";
    // // Send data over mesh network
    // mesh.sendSingle(to, msg);

    StaticJsonDocument<200> doc;
    doc["type"] = "data";
    doc["nodeId"] = nodeName;
    // Read name from EEPROM
    // char name[EEPROM_SIZE - NODE_NAME];
    // for(int i = 0; i < EEPROM_SIZE - NODE_NAME; i++) {
    //   name[i] = EEPROM.read(NODE_NAME + i);
    //   // break if null terminator is found
    //   if(name[i] == '\0') {
    //     break;
    //   }
    // }
    doc["name"] = name_buffer;

    String jsonString;
    serializeJson(doc, jsonString);
    mesh.sendSingle(to, jsonString);

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