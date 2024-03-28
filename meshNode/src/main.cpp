#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cstring>
#include <TaskScheduler.h>
// #include <string>
// #include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <DHT.h> // Include the DHT library
// Include namedMesh.h from ./lib
// #include "sensor.h"
// #include "namedMesh.h"
#include "eepromManager.h"
#include "sensorManager.h"

// Mesh configuration
// Scheduler userScheduler; // to control mesh tasks
// #define   MESH_SSID       "plantware-mesh"
// #define   MESH_PASSWORD   "hest-67890"
// #define   MESH_PORT       5555
// namedMesh  mesh;
// String nodeName; // TODO: Check if this is necessary
// String to = "prov";

// EEPROMManager configuration
// UserEEPROMManager eepromManager;
// #define NODE_ID_TRUE (1 << 0)
// #define SENSOR_1_TRUE (1 << 1)
// #define SENSOR_2_TRUE (1 << 2)
// #define SENSOR_3_TRUE (1 << 3)
// #define SENSOR_4_TRUE (1 << 4)

// // Sensor configuration
SensorManager sensorManager;

// Send message types
enum SndMessageType {
  HELLO,
  SENSORHELLO,
  NAME,
  DATA
};

// Function declarations
// void meshInit();
void sensorTypeInit();
String createJsonString(float temperature_f, float temperature_c, float humidity);
void dhtReadTask(void *pvParameters);
void dataTask(void *pvParameters);

// Setup
void setup() {

  // Start serial monitoring for debugging
  Serial.begin(115200);

  // // Initialize the mesh network
  // meshInit();

  // Initialize the EEPROM
  EEPROM.begin(EEPROM_SIZE);

  delay(3000);
  // Check that the sensors are the same after boot
  sensorTypeInit();

  // Create new sensor factory
  // TODO: THIS NEEDS TO BE CHANGED
  // SensorFactory* sensorFactory = new DHTSensorFactory();
  // sensor1 = sensorFactory->createSensor(SENSOR_1_TYPE, SENSOR_1_PIN);
}

// Loop (most code should be in tasks)
void loop() {
  // mesh.update();
  // sensorManager.getSingleSensorType(1);
  // Serial.printf("Sensor 1 type: %d\n", sensorManager.returnSensorType(1));
  // delay(1000);
  // sensorManager.getSingleSensorType(2);
  // Serial.printf("Sensor 2 type: %d\n", sensorManager.returnSensorType(2));
  // delay(1000);
  // sensorManager.getSingleSensorType(3);
  // Serial.printf("Sensor 3 type: %d\n", sensorManager.returnSensorType(3));
  // delay(1000);
  // sensorManager.getSingleSensorType(4);
  // Serial.printf("Sensor 4 type: %d\n", sensorManager.returnSensorType(4));
  // delay(1000);
}

void startSensorTask() {
  xTaskCreate(dataTask,      // Task function
              "DataTask",    // Task name
              10000,             // Stack size
              NULL,              // Task parameters
              1,                 // Task priority
              NULL);             // Task handle
}

void sensorTypeInit() {
  String sensorType = "";
  int totalChangeFlag = 0;
  int currentSensorType = -1;
  int eepromSensorType = -1;
  // Read the change flags
  readChangeFlags(); // Always has to happen first, will ensure this later
  readSensorTypes();
  // Read the sensor IDS
  readSensorIDs();

  // Loop through all sensors
  for(int i = 1; i < 5; i++) {
    currentSensorType = sensorManager.returnSensorType(i);
    Serial.printf("Sensor %d type: %d\n", i, currentSensorType);
    // Check if the sensor type has been stored in EEPROM before
    if (returnChangeFlag(i)) {
      Serial.printf("Sensor %d has been stored in EEPROM before\n", i);
      // Read the sensor type from EEPROM
      eepromSensorType = returnSensorType(i);
      // Check if the sensor type is the same as the one stored in EEPROM
      if(currentSensorType != eepromSensorType) {
        // Store the new sensor type in RAM and EEPROM
        setSingleSensorType(i, currentSensorType);
        
        // Send a hello message to update the sensor type on the backend
        Serial.printf("Sensor %d type has changed from %d to %d\n", i, eepromSensorType, currentSensorType);

      }
      else {
        // No hello message necessary, can start sending from the same sensor ID
        Serial.printf("Sensor %d type has not changed\n", i);
        Serial.printf("No need to send a hello message\n");
        // Load the sensor ID from EEPROM
      }
    }
    else if (currentSensorType == UNKNOWN_SENSOR) {
      Serial.printf("Sensor %d is unknown\n", i);
      Serial.printf("No need to send a hello message\n");
    }
    else {
      // Store the sensor type in EEPROM
      Serial.printf("Sensor %d has not been stored in EEPROM before\n", i);
      // Store the sensor type in EEPROM
      setSingleSensorType(i, currentSensorType);
      // Send a hello message to update the sensor type on the backend
      Serial.printf("Sensor %d type has been stored in EEPROM\n", i);
      Serial.printf("Sensor %d type has been sent to the backend\n", i);
    }
  }  
}

// void sendMessageHandler(String msg, SndMessageType type) {
//   switch (type) {
//     case HELLO: 
//     {
//       // Send message containing node ID and current Name
//       StaticJsonDocument<200> doc;
//       doc["type"] = "hello";
//       doc["nodeId"] = mesh.getNodeId();

//       // Check bit 0 of the change flag to see if Node ID has been set
//       if(eepromManager.getChangeFlag() & NODE_ID_TRUE) {
//         // TODO: CHANGE THE ID TO PASS INTS NOT STRINGS!!!
//         doc["name"] = String(eepromManager.getID(NODE_ID_TYPE));
//         doc["putTrue"] = 0;

//       } 
//       else {
//         // doc["name"] = mesh.getName();
//         doc["putTrue"] = 1;
//       }


//       // if(ncf == 1) {
//       //   doc["name"] = name_buffer;
//       // }
//       // // If no name is stored in EEPROM, set name to node ID
//       // else {
//       //   doc["name"] = mesh.getName();
//       // }

//       // Send hello message
//       String jsonString;
//       serializeJson(doc, jsonString);
//       Serial.println("Sending hello message");
//       // Print the message
//       Serial.println(jsonString);
//       mesh.sendSingle(to, jsonString);
      
//       break;
//     }
//     case SENSORHELLO:
//     {
//       // Check if sensor1 is registered
//       if(eepromManager.getChangeFlag() & SENSOR_1_TRUE){
//         // Send message containing node ID and current Name
//         StaticJsonDocument<200> doc;
//         doc["type"] = "sensorhello";
//         doc["nodeId"] = mesh.getNodeId();

//         // Check if there is a name already stored in EEPROM
//         // TODO: UPDATE THIS TO CHECK IF THE DBID IS ALREADY STORED IN EEPROM
//         doc["plantId"] = String(eepromManager.getID(SENSOR_1_TYPE));
//         doc["name"] = "sensor1";

//         String jsonString;
//         serializeJson(doc, jsonString);
//         Serial.println("Sending sensorhello message");
//         // Print the message
//         Serial.println(jsonString);
//         mesh.sendSingle(to, jsonString);
//       }
//       break;
//     }
//     case NAME:
//     {
//       Serial.printf("Unused type: %d\n", type);
//       break;
//     }
//     case DATA:
//     {
//       // mesh.sendSingle(to, msg);
//       // Deserialize the message
//       StaticJsonDocument<200> docIn;
//       deserializeJson(docIn, msg);
      
//       StaticJsonDocument<200> doc;
      
//       // Add data header to the message
//       doc["type"] = "data";
//       // // Read the sensor1 ID from EEPROM
//       // char sensor1_id[EEPROM_UNIT_SIZE];
//       // for(int i = 0; i < EEPROM_UNIT_SIZE - SENSOR_1; i++) {
//       //   sensor1_id[i] = EEPROM.read(SENSOR_1 + i);
//       //   // break if null terminator is found
//       //   if(sensor1_id[i] == '\0') {
//       //     break;
//       //   }
//       // }
//       doc["sensorId"] = eepromManager.getID(SENSOR_1_TYPE);

//       // Nest the data in a data object
//       // JsonObject data = doc.createNestedObject("data");
//       StaticJsonDocument<200> data;
//       data["temperatureF"] = docIn["temperatureF"];
//       data["temperatureC"] = docIn["temperatureC"];
//       data["humidity"] = docIn["humidity"];

//       String jsonStringData;
//       serializeJson(data, jsonStringData);
//       doc["data"] = jsonStringData;

//       // Send the data message
//       String jsonString;
//       serializeJson(doc, jsonString);
//       mesh.sendSingle(to, jsonString);

//       break;
//     }
//     default:
//       break;
//   }
// }

// void receiveMessageHandler(String msg) {
//   // Deserialize the message
//   StaticJsonDocument<200> doc;
//   deserializeJson(doc, msg);

//   // Check the type of message
//   const char* type = doc["type"];
//   if(strcmp(type, "config") == 0) {
//     // Store the name in EEPROM
//     Serial.println("Received config message");
//     const char* name = doc["name"];
//     // TODO: ADD CHECK HERE TO MAKE SURE THE LENGTH IS NOT LONGER THAN THE EEPROM
//     eepromManager.save(NODE_ID_TYPE, name);
//     // Send another hello message to update the name
//     sendMessageHandler("", HELLO);

//     // Wait 5 seconds
//     vTaskDelay(pdMS_TO_TICKS(5000));

//     // Send sensor hello message
//     sendMessageHandler("", SENSORHELLO);
//   }
//   else if(strcmp(type, "sensorConfig") == 0) {
//     // Store the sensor ID in EEPROM
//     Serial.println("Received sensorConfig message");
//     const char* sensorID = doc["sensorId"];
//     eepromManager.save(SENSOR_1_TYPE, sensorID);
//     // Delay for 5 seconds
//     vTaskDelay(pdMS_TO_TICKS(5000));
//     startSensorTask();
//   }
// }

// void meshInit() {
//   // Setup the mesh network
//   mesh.setDebugMsgTypes(ERROR);  // set before init() so that you can see startup messages
//   mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

//   // Set mesh node name to the node id
//   nodeName = String(mesh.getNodeId());
//   mesh.setName(nodeName); // This needs to be an unique name! 

//   // Handle message receiving
//   // mesh.onReceive([](uint32_t from, String &msg) {
//   //   Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
//   // });
//   mesh.onReceive([](String &from, String &msg) {
//     Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
//     receiveMessageHandler(msg);
//   });

//   // Handle changed connections
//   mesh.onChangedConnections([]() {
//     Serial.printf("Changed connection\n");
//     // Wait 5 seconds before sending hello message
//     vTaskDelay(pdMS_TO_TICKS(5000));
//     // Send hello type message
//     sendMessageHandler("", HELLO);
//   });
// }

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
    StaticJsonDocument<200> sensorData;

    // Read sensor 1
    // sensorData = sensorManager.readSensorData(SENSOR_1_TYPE);
    
    // Wait for 5 seconds before the next message
    vTaskDelay(pdMS_TO_TICKS(15000));
  }
}

// // Put sensor data into a JSON string
// String createJsonString(float temperature_f, float temperature_c, float humidity) {
//   StaticJsonDocument<200> doc;

//   doc["nodeId"] = nodeName;
//   doc["temperatureF"] = temperature_f;
//   doc["temperatureC"] = temperature_c;
//   doc["humidity"] = humidity;

//   String jsonString;
//   serializeJson(doc, jsonString);

//   return jsonString;
// }