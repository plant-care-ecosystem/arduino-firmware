#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cstring>
// #include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <DHT.h> // Include the DHT library
// Include namedMesh.h from ./lib
#include "sensor.h"
#include "namedMesh.h"
#include "eepromManager.h"

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

// EEPROMManager configuration
EEPROMManager eepromManager;
#define NODE_ID_TRUE (1 << 0)
#define SENSOR_1_TRUE (1 << 1)
#define SENSOR_2_TRUE (1 << 2)
#define SENSOR_3_TRUE (1 << 3)
#define SENSOR_4_TRUE (1 << 4)

// Send message types
enum SndMessageType {
  HELLO,
  SENSORHELLO,
  NAME,
  DATA
};

// Function declarations
void meshInit();
String createJsonString(float temperature_f, float temperature_c, float humidity);
void dhtReadTask(void *pvParameters);
void dataTask(void *pvParameters);


// Setup
void setup() {
  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Initialize the mesh network
  meshInit();

  // Load the EEPROM
  eepromManager.load();

  // Create new sensor factory
  // TODO: THIS NEEDS TO BE CHANGED
  SensorFactory* sensorFactory = new DHTSensorFactory();
  sensor1 = sensorFactory->createSensor(SENSOR_1_TYPE, SENSOR_1_PIN);
}

// Loop (most code should be in tasks)
void loop() {
  mesh.update();
}

void startSensorTask() {
  xTaskCreate(dataTask,      // Task function
              "DataTask",    // Task name
              10000,             // Stack size
              NULL,              // Task parameters
              1,                 // Task priority
              NULL);             // Task handle
}

void sendMessageHandler(String msg, SndMessageType type) {
  switch (type) {
    case HELLO: 
    {
      // Send message containing node ID and current Name
      StaticJsonDocument<200> doc;
      doc["type"] = "hello";
      doc["nodeId"] = mesh.getNodeId();

      // Check bit 0 of the change flag to see if Node ID has been set
      if(eepromManager.getChangeFlag() & NODE_ID_TRUE) {
        // TODO: CHANGE THE ID TO PASS INTS NOT STRINGS!!!
        doc["name"] = String(eepromManager.getID(NODE_ID_TYPE));
        doc["putTrue"] = 0;

      } 
      else {
        // doc["name"] = mesh.getName();
        doc["putTrue"] = 1;
      }


      // if(ncf == 1) {
      //   doc["name"] = name_buffer;
      // }
      // // If no name is stored in EEPROM, set name to node ID
      // else {
      //   doc["name"] = mesh.getName();
      // }

      // Send hello message
      String jsonString;
      serializeJson(doc, jsonString);
      Serial.println("Sending hello message");
      // Print the message
      Serial.println(jsonString);
      mesh.sendSingle(to, jsonString);
      
      break;
    }
    case SENSORHELLO:
    {
      // Check if sensor1 is registered
      if(eepromManager.getChangeFlag() & SENSOR_1_TRUE){
        // Send message containing node ID and current Name
        StaticJsonDocument<200> doc;
        doc["type"] = "sensorhello";
        doc["nodeId"] = mesh.getNodeId();

        // Check if there is a name already stored in EEPROM
        // TODO: UPDATE THIS TO CHECK IF THE DBID IS ALREADY STORED IN EEPROM
        doc["plantId"] = String(eepromManager.getID(SENSOR_1_TYPE));
        doc["name"] = "sensor1";

        String jsonString;
        serializeJson(doc, jsonString);
        Serial.println("Sending sensorhello message");
        // Print the message
        Serial.println(jsonString);
        mesh.sendSingle(to, jsonString);
      }
      break;
    }
    case NAME:
    {
      Serial.printf("Unused type: %d\n", type);
      break;
    }
    case DATA:
    {
      // mesh.sendSingle(to, msg);
      // Deserialize the message
      StaticJsonDocument<200> docIn;
      deserializeJson(docIn, msg);
      
      StaticJsonDocument<200> doc;
      
      // Add data header to the message
      doc["type"] = "data";
      // // Read the sensor1 ID from EEPROM
      // char sensor1_id[EEPROM_UNIT_SIZE];
      // for(int i = 0; i < EEPROM_UNIT_SIZE - SENSOR_1; i++) {
      //   sensor1_id[i] = EEPROM.read(SENSOR_1 + i);
      //   // break if null terminator is found
      //   if(sensor1_id[i] == '\0') {
      //     break;
      //   }
      // }
      doc["sensorId"] = eepromManager.getID(SENSOR_1_TYPE);

      // Nest the data in a data object
      // JsonObject data = doc.createNestedObject("data");
      StaticJsonDocument<200> data;
      data["temperatureF"] = docIn["temperatureF"];
      data["temperatureC"] = docIn["temperatureC"];
      data["humidity"] = docIn["humidity"];

      String jsonStringData;
      serializeJson(data, jsonStringData);
      doc["data"] = jsonStringData;

      // Send the data message
      String jsonString;
      serializeJson(doc, jsonString);
      mesh.sendSingle(to, jsonString);

      break;
    }
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
    const char* name = doc["name"];
    // TODO: ADD CHECK HERE TO MAKE SURE THE LENGTH IS NOT LONGER THAN THE EEPROM
    eepromManager.save(NODE_ID_TYPE, name);
    // Send another hello message to update the name
    sendMessageHandler("", HELLO);

    // Wait 5 seconds
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Send sensor hello message
    sendMessageHandler("", SENSORHELLO);
  }
  else if(strcmp(type, "sensorConfig") == 0) {
    // Store the sensor ID in EEPROM
    Serial.println("Received sensorConfig message");
    const char* sensorID = doc["sensorId"];
    eepromManager.save(SENSOR_1_TYPE, sensorID);
    // Delay for 5 seconds
    vTaskDelay(pdMS_TO_TICKS(5000));
    startSensorTask();
  }
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
    // Wait 5 seconds before sending hello message
    vTaskDelay(pdMS_TO_TICKS(5000));
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
void dataTask(void *pvParameters) {
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
    doc["temperatureF"] = sensor1->getTemperatureF();
    doc["temperatureC"] = sensor1->getTemperatureC();
    doc["humidity"] = sensor1->getHumidity();

    String jsonString;
    serializeJson(doc, jsonString);
    sendMessageHandler(jsonString, DATA);
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