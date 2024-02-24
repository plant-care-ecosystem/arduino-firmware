#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
// Include namedMesh.h from ./lib
#include "namedMesh.h"

// Mesh configuration
Scheduler userScheduler; // to control mesh tasks
#define   MESH_SSID       "plantware-mesh"
#define   MESH_PASSWORD   "hest-67890"
#define   MESH_PORT       5555
namedMesh  mesh;
String nodeName = "prov";

// Function declarations

// Setup
void setup() {
  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Setup the mesh network
  mesh.setDebugMsgTypes(ERROR);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](uint32_t from, String &msg) {
    Serial.printf("Received message by id from: %u, %s\n", from, msg.c_str());
  });

  mesh.onReceive([](String &from, String &msg) {
    Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    Serial.printf("Changed connection\n");
  });
}

// Loop (most code should be in tasks)
void loop() {
  mesh.update();
}