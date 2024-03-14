
#include <Arduino.h>
#include <WiFi.h>
#include <HardwareSerial.h>
#include <map>
#include <cstring>
#include "namedMesh.h" // Include namedMesh.h from ./lib

// Map configuration
std::map<uint32_t, String> nodeIdToName;

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


void sendNameToNode(String currentName, String newName) {
  StaticJsonDocument<200> doc;
  doc["type"] = "config";
  doc["name"] = newName;

  // Convert JSON doc into sendable string
  String jsonString;
  serializeJson(doc, jsonString);

  Serial.printf("Sending new name to node: %s\n", currentName.c_str());

  mesh.sendSingle(currentName, jsonString);
}

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
    String nodeName = doc["name"];
    if (String(nodeId) == nodeName) {
      Serial.println("Node ID and name are the same");
      // Create a new name based on how many nodes are provisioned
      String newNodeName = "plant-" + String(mesh.getNodeList().size());
      // Print new node name
      Serial.printf("New node name: %s\n", newNodeName.c_str());
      // Send the new name to the node
      sendNameToNode(nodeName, newNodeName);
    } else {
      // Store node name in map
      nodeIdToName[nodeId] = nodeName;
    }
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