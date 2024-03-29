
#include <Arduino.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include <map>
#include <cstring>
#include "namedMesh.h" // Include namedMesh.h from ./lib

// Map configuration
std::map<uint32_t, uint32_t> nodeIdToName;

// Mesh configuration
Scheduler userScheduler;           // To control mesh tasks
#define MESH_SSID "plantware-mesh" // Mesh network name
#define MESH_PASSWORD "hest-67890" // Mesh network password
#define MESH_PORT 5555             // Mesh network port
namedMesh mesh;
String nodeName = "prov"; // Provisioner should always be named "prov"

// Serial communication configuration
#define UART1_TX 5             // Pin 5
#define UART1_RX 4             // Pin 4
HardwareSerial SerialPort1(1); // Start serial port 1

// Message types
enum MessageType {
  DATA,
  CONFIG,
  UNKNOWN
};
// Function declarations ------------------------------------------------------

/*
 * Start serial communication
 *
 * Both for debugging and sending data to poster slave
 */
void serialInit();

void sendNameToNode(String currentName, String newName);

void handleMessage(String &from, String &msg);

/*
 * Initialize the mesh network
 *
 * Start the mesh network and set the debug message types.
 * Set the mesh network name and handle new connections,
 * dropped connections, message receiving and changed connections.
 */
void meshInit();

void sendToPoster(String message);

// void readSerialInputTask(void * parameter) {
//   for (;;) {
//     if (Serial.available()) {
//       String input = Serial.readStringUntil('\n');
//       Serial.printf("Current name: %s\n", input.c_str());
//       StaticJsonDocument<200> doc;
//       DeserializationError error = deserializeJson(doc, input);
//       if (error) {
//         Serial.print(F("deserializeJson() failed: "));
//         Serial.println(error.c_str());
//         return;
//       }
//       String newName = doc["newName"];
//       Serial.printf("New name: %s\n", newName.c_str());
//       String currentName = doc["name"];
//       Serial.printf("Name: %s\n", currentName.c_str());
//       sendNameToNode(currentName, newName);
      
//     }
//     vTaskDelay(1); // Delay for task switching
//   }
// }

void parseSerial(void *pvParameters);

// Setup ----------------------------------------------------------------------
void setup() {

  // Start serial communication
  serialInit();

  // Start mesh network
  meshInit();

  // // Create a task to print the map
  // xTaskCreate([](void *pvParameters) {
  //   for (;;) {
  //     Serial.println("Node ID to name map:");
  //     for (auto it = nodeIdToName.begin(); it != nodeIdToName.end(); ++it) {
  //       Serial.printf("Node ID: %u, Name: %s\n", it->first, it->second.c_str());
  //     }
  //     vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay for 10 seconds
  //   }
  // }, "printMap", 10000, NULL, 2, NULL);

  // xTaskCreate(readSerialInputTask, "ReadSerialInput", 10000, NULL, 1, NULL);

        // FreeRTOS task for handling serial port 1
  xTaskCreate(parseSerial,   // Function to run
              "SerialPort1Task", // Name of the task
              10000,             // Stack size (bytes)
              NULL,              // Parameter to pass
              1,                 // Task priority
              NULL               // Task handle
  );
  
}

// Loop (most code should be in tasks)
void loop() {
  mesh.update();
}

// Start serial communication
void serialInit() {
  // Start serial monitoring for debugging
  Serial.begin(115200);
  // Start serial port for sending data to poster slave
  SerialPort1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
}

MessageType getMessageType(const char* type) {
  if (strcmp(type, "data") == 0) {
    return MessageType::DATA;
  } else if (strcmp(type, "config") == 0) {
    return MessageType::CONFIG;
  } else {
    return MessageType::UNKNOWN;
  }
}

void sendInitialIdToNode(String nodeId, String dbId) {
  StaticJsonDocument<200> doc;
  doc["type"] = "config";
  doc["name"] = dbId;

  // Convert JSON doc into sendable string
  String jsonString;
  serializeJson(doc, jsonString);
  mesh.sendSingle(nodeId, jsonString);
}

void sendSensorIdToNode(String nodeId, String dbId, int sensorPort) {
  StaticJsonDocument<200> doc;
  doc["type"] = "sensorConfig";
  doc["sensorId"] = dbId;
  doc["sensorNumber"] = sensorPort;

  // Convert JSON doc into sendable string
  String jsonString;
  serializeJson(doc, jsonString);
  mesh.sendSingle(nodeId, jsonString);
}

// void sendNameToNode(String currentName, String newName) {
//   StaticJsonDocument<200> doc;
//   doc["type"] = "config";
//   doc["name"] = newName;
//   // Search for the node ID based on the name
//   uint32_t nodeId;
//   for (auto it = nodeIdToName.begin(); it != nodeIdToName.end(); ++it) {
//     if (it->second == currentName) {
//       nodeId = it->first;
//       break;
//     }
//   }
//   currentName = String(nodeId);
  

//   // Convert JSON doc into sendable string
//   String jsonString;
//   serializeJson(doc, jsonString);

//   Serial.printf("Sending new name to node: %s\n", currentName.c_str());

//   mesh.sendSingle(currentName, jsonString);
// }

void handleMessage(String &from, String &msg) {
  // Print the received message for debugging
  Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  
  // Parse the JSON message
  StaticJsonDocument<200> doc;

  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  // Get the message type
  const char* type = doc["type"];

  if(strcmp(type, "hello") == 0){
    Serial.println("Hello message received");
    // Check if nodeId and name are both the ID (uninitialized node)
    uint32_t nodeId = doc["nodeId"];
    uint32_t putTrue = doc["putTrue"];
    Serial.printf("Node ID: %u, putTrue: %u\n", nodeId, putTrue);
    if (putTrue == 1) {
      Serial.println("Node needs to be put in the server");
      // Send the plant name for the node to the poster
      StaticJsonDocument<200> doc;
      doc["type"] = "config";
      doc["nodeId"] = nodeId;
      doc["name"] = String(nodeId);

      // Convert JSON doc into sendable string
      String jsonString;
      serializeJson(doc, jsonString);
      sendToPoster(jsonString);
      // TODO: ADD userID to the MESSAGE ON THE POSTER SIDE

    } else {
      // Store node name in map
      uint32_t nodeName = doc["name"];
      nodeIdToName[nodeId] = nodeName;
      // Print the map for debugging
      // Serial.println("Node ID to name map:");
      for (auto it = nodeIdToName.begin(); it != nodeIdToName.end(); ++it) {
        Serial.printf("Node ID: %u, Name: %u\n", it->first, it->second);
      }

      
    }
  }
  else if (strcmp(type, "sensorhello") == 0) {
    Serial.println("Sensor hello message received");
    sendToPoster(msg);
  }
  else if (strcmp(type, "data") == 0) {
    Serial.println("Data message received");
    // Send the message to the poster
    sendToPoster(msg);
  }
  else if (strcmp(type, "config") == 0) {
    Serial.println("Config message received");
    // Send the message to the poster
    sendToPoster(msg);
  }
  else {
    Serial.println("Unknown message type");
  }
  // return;
}

// Initialize the mesh network
void meshInit() {
  // Initialize the mesh network using SSID and password
  mesh.setDebugMsgTypes(ERROR);  // set before init() so that you can see startup messages if necessary
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  // Set provisioner mesh node (this device) name to "prov"
  mesh.setName(nodeName); // This needs to be an unique name! 

  // Handle removed connections and remove a name based on how many are connected
  mesh.onDroppedConnection([](uint32_t nodeId) {
    Serial.printf("Node dropped: %u\n", nodeId);
    // Remove the node from the name map
    nodeIdToName.erase(nodeId);
  });

  // Handle message receiving
  // mesh.onReceive([](uint32_t from, String &msg) {
  //   Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  //   // TODO: APPEND NODE ID AND NAME TO THE MESSAGE
  //   // use the map to find the name from the ID??
  //   sendToPoster(msg);
  // });
  // mesh.onReceive([](String &from, String &msg) {
  //   Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());

  // });

  mesh.onReceive([](String &from, String &msg) {
    handleMessage(from, msg);
  });

  // Handle changed connections
  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
    // Send the node name to the new node
    // String nodeName = "plant-" + String(mesh.getNodeList().size());
    // StaticJsonDocument<200> doc;
    // doc["type"] = "config";
    // doc["name"] = nodeName;

    // // Convert JSON doc into sendable string
    // String jsonString;
    // serializeJson(doc, jsonString);

    // mesh.sendSingle(jsonString, nodeName);
  });
}

void sendToPoster(String message) {
  SerialPort1.println(message); 
}

// // Function to parse serial data
void parseSerial(void *pvParameters) {
  for (;;) {
    if (SerialPort1.available()) {
      String input = SerialPort1.readStringUntil('\n');
      Serial.println(input);
      StaticJsonDocument<200> doc;
      deserializeJson(doc, input);
      String type = doc["type"];

      // Check for message type
      if(type == "plantadd") {
        Serial.println("Plant add message received");
        sendInitialIdToNode(doc["nodeId"], doc["dbId"]);
      }
      if(type == "sensoradd") {
        Serial.println("Sensor add message received");
        sendSensorIdToNode(doc["nodeId"], doc["dbId"]);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10)); // Delay for task switching
  }
  
}