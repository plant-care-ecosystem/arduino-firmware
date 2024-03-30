#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cstring>
// #include <TaskScheduler.h>
// #include <string>
// #include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <DHT.h> // Include the DHT library
// Include namedMesh.h from ./lib
// #include "sensor.h"
#include "namedMesh.h"
// #include "eepromManager.h"
#include "sensorManager.h"

// Mesh configuration
Scheduler userScheduler; // to control mesh tasks
#define   MESH_SSID       "plantware-mesh"
#define   MESH_PASSWORD   "hest-67890"
#define   MESH_PORT       5555
namedMesh  mesh;
String nodeName; // TODO: Check if this is necessary
String to = "prov";

// Communication flags
int nodeIDRcvFlag = 0;
int sensorSendFlags[4] = {0, 0, 0, 0};

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
void sendMessageHandler(String msg, SndMessageType type, int sensorNumber = -1);
void receiveMessageHandler(String msg);
void sensorTypeInit();
String createJsonString(float temperature_f, float temperature_c, float humidity);
// void dhtReadTask(void *pvParameters);
Task dataTask(15000, TASK_FOREVER, [](){
  StaticJsonDocument<200> sensorData;
    int lastSensorType;
    for(int i = 1; i < 5; i++) {
      // Record current sensor type as last sensor type
      lastSensorType = sensorManager.returnSensorType(i);
      // Get the single sensor type from the resistor value
      sensorManager.getSingleSensorType(i);
      // Check the current sensor type is the same as stored in ram
      Serial.printf("Last sensor type: %d\n", lastSensorType);
      Serial.printf("Sensor %d type: %d\n", i, sensorManager.returnSensorType(i));

      // if(lastSensorType != sensorManager.returnSensorType(i)) {
      //   // Store the new sensor type in EEPROM
      //   setSingleSensorType(i, sensorManager.returnSensorType(i));
        
      //   // Send a hello message to update the sensor type on the backend
      //   sendMessageHandler("", SENSORHELLO, i);

      //   delay(15000);
      // }
      // Read the sensor data
      // Check that sensor is not unknown 
      if(sensorManager.returnSensorType(i) == UNKNOWN_SENSOR) {
        Serial.printf("Sensor %d is unknown\n", i);
      }
      else {
        sensorData = sensorManager.getSensorData(i);
        // Serialize the data
        String jsonString;
        serializeJson(sensorData, jsonString);
        // Send the data
        sendMessageHandler(jsonString, DATA, i);
      }
    }
});

// Send hello message every 30 seconds until node ack is received
Task sendHelloTask(30000, TASK_FOREVER, []() {
  if (nodeIDRcvFlag == 1) {
    return;
  }
  else {
    sendMessageHandler("", HELLO);
  }
  
});

// Setup
void setup() {

  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Initialize the EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Read the change flags
  readChangeFlags(); // Always has to happen first, will ensure this later
  readSensorTypes();
  // Read the sensor IDS
  readSensorIDs();


  mesh.setDebugMsgTypes(ERROR);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  // Set the name to the node ID
  nodeName = String(mesh.getNodeId());

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    // Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
    receiveMessageHandler(msg);
  });

  // mesh.onReceive([](uint32_t from, String &msg) {
  //   Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  // });

  // mesh.onReceive([](String &from, String &msg) {
  //   Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  // });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");

    // Wait 5 seconds before sending hello message
    // delay(5000);
    // Send hello type message
    // while(nodeIDRcvFlag == 0) {
      // sendMessageHandler("", HELLO);
      // // vTaskDelay(pdMS_TO_TICKS(15000));
      // Serial.println("Waiting for node ID ack to be received\n");
      
  });

  userScheduler.addTask(sendHelloTask);
  sendHelloTask.enable();


  // Create new sensor factory
  // TODO: THIS NEEDS TO BE CHANGED
  // SensorFactory* sensorFactory = new DHTSensorFactory();
  // sensor1 = sensorFactory->createSensor(SENSOR_1_TYPE, SENSOR_1_PIN);
}

// Loop (most code should be in tasks)
void loop() {
  mesh.update();
}

void startSensorTask() {
  // xTaskCreate(dataTask,      // Task function
  //             "DataTask",    // Task name
  //             10000,             // Stack size
  //             NULL,              // Task parameters
  //             1,                 // Task priority
  //             NULL);             // Task handle
  userScheduler.addTask(dataTask);
  dataTask.enable();
}

void sendMessageHandler(String msg, SndMessageType type, int sensorNumber) {
  switch (type) {
    case HELLO: 
    {
      // Send message containing node ID and current Name
      StaticJsonDocument<200> doc;
      doc["type"] = "hello";
      doc["nodeId"] = mesh.getNodeId();

      // Check bit 0 of the change flag to see if Node ID has been set
      if(nodeIDChangeFlag) {
        // TODO: CHANGE THE ID TO PASS INTS NOT STRINGS!!!
        doc["name"] = nodeID;
        doc["putTrue"] = 0;

      } 
      else {
        doc["name"] = mesh.getName();
        doc["putTrue"] = 1;
      }

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
      StaticJsonDocument<200> doc;
      doc["type"] = "sensorhello";
      doc["nodeId"] = mesh.getNodeId();
      // Check which sensor is sending the message
      if(sensorNumber == 1) {
        Serial.println("Hello message from Sensor 1\n");
        doc["plantId"] = nodeID;
        doc["sensorNumber"] = 1;
        // Send the hello message
        doc["name"] = sensorManager.generateSensorName(sensor1Type, 1);
      }
      else if(sensorNumber == 2) {
        Serial.println("Hello message from Sensor 2\n");
        doc["plantId"] = nodeID;
        doc["sensorNumber"] = 2;
        // Send the hello message
        doc["name"] = sensorManager.generateSensorName(sensor2Type, 2);
      }
      else if(sensorNumber == 3) {
        Serial.println("Hello message from Sensor 3\n");
        doc["plantId"] = nodeID;
        doc["sensorNumber"] = 3;
        // Send the hello message
        doc["name"] = sensorManager.generateSensorName(sensor3Type, 3);
      }
      else if(sensorNumber == 4) {
        Serial.println("Hello message from Sensor 4\n");
        doc["plantId"] = nodeID;
        doc["sensorNumber"] = 4;
        // Send the hello message
        doc["name"] = sensorManager.generateSensorName(sensor4Type, 4);
      }
      else {
        Serial.println("Hello message from unknown sensor\n");
        break;
      }
      String jsonString;
      serializeJson(doc, jsonString);
      Serial.printf("Sending sensor hello message: %s\n", jsonString.c_str());
      mesh.sendSingle(to, jsonString);
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
      if(sensorNumber == 1) {
        doc["sensorId"] = sensor1ID;
      }
      else if(sensorNumber == 2) {
        doc["sensorId"] = sensor2ID;
      }
      else if(sensorNumber == 3) {
        doc["sensorId"] = sensor3ID;
      }
      else if(sensorNumber == 4) {
        doc["sensorId"] = sensor4ID;
      }
      else {
        Serial.println("Unknown sensor number\n");
        break;
      }

      doc["data"] = msg;
      // // Nest the data in a data object
      // // JsonObject data = doc.createNestedObject("data");
      // StaticJsonDocument<200> data;
      // // data["temperatureF"] = docIn["temperatureF"];
      // // data["temperatureC"] = docIn["temperatureC"];
      // // data["humidity"] = docIn["humidity"];

      // // String jsonStringData;
      // // serializeJson(data, jsonStringData);
      // // doc["data"] = jsonStringData;
      // data = sensorManager.readSensorData(SENSOR_1_TYPE);

      // // Send the data message
      String jsonString;
      serializeJson(doc, jsonString);
      Serial.printf("Sending data message: %s\n", jsonString.c_str());
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
    String idString = doc["name"];
    // TODO: ADD CHECK HERE TO MAKE SURE THE LENGTH IS NOT LONGER THAN THE EEPROM
    nodeID = idString.toInt();
    Serial.printf("Node ID: %d\n", nodeID);
    // Serial.printf("Node ID String: %s\n", );
    // add \0 to the end of the string
    
    EEPROM.put(NODE_ID, idString);
    EEPROM.commit();

    nodeIDChangeFlag = 1;
    EEPROM.put(NODE_ID_CHANGE_FLAG, 1);
    EEPROM.commit();

    nodeIDRcvFlag = 1;

    // Wait 5 seconds
    // vTaskDelay(pdMS_TO_TICKS(5000));

    // Send another hello message to update the name
    sendMessageHandler("", HELLO);
  }
  else if(strcmp(type, "sensorConfig") == 0) {
    // Store the sensor ID in EEPROM
    Serial.println("Received sensorConfig message");
    // Print the received message
    Serial.printf("Message: %s\n", msg.c_str());
    // Serial.println("Message: %s\n", msg.c_str());
    int sensorNumber = doc["sensorNumber"];
    switch(sensorNumber) {
      case 1:
      {
        // Store the sensor ID in EEPROM
        String sensorID = doc["sensorId"];
        Serial.printf("Sensor 1 ID: %s\n", sensorID.c_str());
        sensor1ID = sensorID.toInt();
        sensor1ChangeFlag = 1;
        EEPROM.put(SENSOR_1_ID, sensorID);
        EEPROM.commit();
        EEPROM.put(SENSOR_1_CHANGE_FLAG, 1);
        EEPROM.commit();
        sensorSendFlags[0] = 0;
        // Set sensor 1 rcv flag to high
        // sensor1RcvFlag = 1;
        break;
      }
      case 2:
      {
        // Store the sensor ID in EEPROM
        String sensorID = doc["sensorId"];
        sensor2ID = sensorID.toInt();
        sensor2ChangeFlag = 1;
        Serial.printf("Sensor 2 ID: %s\n", sensorID);
        EEPROM.put(SENSOR_2_ID, sensorID);
        EEPROM.commit();
        EEPROM.put(SENSOR_2_CHANGE_FLAG, 1);
        EEPROM.commit();
        sensorSendFlags[1] = 0;
        // Set sensor 2 rcv flag to high
        // sensor2RcvFlag = 1;
        break;
      }
      case 3:
      {
        // Store the sensor ID in EEPROM
        String sensorID = doc["sensorId"];
        sensor3ID = sensorID.toInt();
        sensor3ChangeFlag = 1;
        Serial.printf("Sensor 3 ID: %s\n", sensorID);
        EEPROM.put(SENSOR_3_ID, sensorID);
        EEPROM.commit();
        EEPROM.put(SENSOR_3_CHANGE_FLAG, 1);
        EEPROM.commit();
        sensorSendFlags[2] = 0;
        // Set sensor 3 rcv flag to high
        // sensor3RcvFlag = 1;
        break;
      }
      case 4:
      {
        // Store the sensor ID in EEPROM
        String sensorID = doc["sensorId"];
        sensor4ID = sensorID.toInt();
        sensor4ChangeFlag = 1;
        Serial.printf("Sensor 4 ID: %s\n", sensorID);
        EEPROM.put(SENSOR_4_ID, sensorID);
        EEPROM.commit();
        EEPROM.put(SENSOR_4_CHANGE_FLAG, 1);
        EEPROM.commit();
        sensorSendFlags[3] = 0;
        // Set sensor 4 rcv flag to high
        // sensor4RcvFlag = 1;
        break;
      }
      default:
        break;
    }
    // Check if all the send flags have been received and processed
    // If so, start the data task
    if(!sensorSendFlags[0] && !sensorSendFlags[1] && !sensorSendFlags[2] && !sensorSendFlags[3]) {
      userScheduler.addTask(dataTask);
      dataTask.enable();
    }
  }
  else if(strcmp(type, "nodeAck") == 0) {
    // Check that the sensors are the same after boot
    // Set node id rcv flag to high
    nodeIDRcvFlag = 1;
    sensorTypeInit();
  }
}

void sensorTypeInit() {
  String sensorType = "";
  int totalChangeFlag = 0;
  int currentSensorType = -1;
  int eepromSensorType = -1;

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

        // Send a hello message to update the sensor type on the backend
        sendMessageHandler("", SENSORHELLO, i);
        Serial.printf("Sensor %d type has been sent to the backend\n", i);
        sensorSendFlags[i - 1] = 1;

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
      // Send a hello message to update the sensor type on the backend
      sendMessageHandler("", SENSORHELLO, i);
      Serial.printf("Sensor %d type has been sent to the backend\n", i);
      sensorSendFlags[i - 1] = 1;
    }
  }  
  if(!sensorSendFlags[0] && !sensorSendFlags[1] && !sensorSendFlags[2] && !sensorSendFlags[3]) {
      userScheduler.addTask(dataTask);
      dataTask.enable();
    }
}

// void meshInit() {
//   Serial.println("Starting mesh network\n");
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
//     // while(nodeIDRcvFlag == 0) {
//       sendMessageHandler("", HELLO);
//       // vTaskDelay(pdMS_TO_TICKS(15000));
//       Serial.println("Waiting for node ID ack to be received\n");
//       delay(15000);
//     // }
    
//   });

//   mesh.onNewConnection([](uint32_t nodeId) {
//     Serial.printf("New connection: %s, %u\n", nodeName.c_str(), nodeId);
    
//     // Wait 5 seconds before sending hello message
//     vTaskDelay(pdMS_TO_TICKS(5000));
//     // Send hello type message
//     // while(nodeIDRcvFlag == 0) {
//       sendMessageHandler("", HELLO);
//       // vTaskDelay(pdMS_TO_TICKS(15000));
//       Serial.println("Waiting for node ID ack to be received\n");
//       delay(15000);
//     // }
//     // Set flag back to 0
//     // nodeIDRcvFlag = 0;

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
// void dataTask(void *pvParameters) {
//   (void)pvParameters; // Unused parameter

//   for (;;) {
//     StaticJsonDocument<200> sensorData;
//     int lastSensorType;
//     for(int i = 1; i < 5; i++) {
//       // Record current sensor type as last sensor type
//       lastSensorType = sensorManager.returnSensorType(i);
//       // Get the single sensor type from the resistor value
//       sensorManager.getSingleSensorType(i);
//       // Check the current sensor type is the same as stored in ram

//       if(lastSensorType != sensorManager.returnSensorType(i)) {
//         // Store the new sensor type in EEPROM
//         setSingleSensorType(i, sensorManager.returnSensorType(i));
        
//         // Send a hello message to update the sensor type on the backend
//         sendMessageHandler("", SENSORHELLO, i);

//         delay(15000);
//       }
//       // Read the sensor data
//       sensorData = sensorManager.getSensorData(i);
//       // Serialize the data
//       String jsonString;
//       serializeJson(sensorData, jsonString);
//       // Send the data
//       sendMessageHandler(jsonString, DATA, i);
//     }
    
//     // Wait for 5 seconds before the next message
//     vTaskDelay(pdMS_TO_TICKS(15000));
//   }
// }

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