#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <cstring>
#include <Adafruit_Sensor.h>
#include <DHT.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
#include "namedMesh.h"
#include "sensorManager.h" // EEPROM manager included in this

// Mesh configuration
Scheduler userScheduler; // to control mesh tasks
#define   MESH_SSID       "plantware-mesh"
#define   MESH_PASSWORD   "hest-67890"
#define   MESH_PORT       5555
namedMesh  mesh;
String nodeName; // TODO: Check if this is necessary
String to = "prov";
String output = "output";

// Communication flags
int nodeIDRcvFlag = 0;
int sensorSendFlags[4] = {0, 0, 0, 0};
int runningFlag = 0;
int globalTaskCount = 1;

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
void dataTask(void *pvParameters);

// // Task reading config
// StaticJsonDocument<200> dataParser() {
//   StaticJsonDocument<200> doc;
//   // Deserialize the message
//   deserializeJson(doc, "config");
//   return doc;
// }


// Create tasks for setting power mux and reading each sensor
void setPowerMux1Callback(); 
Task setPowerMux1Task(0, TASK_ONCE, &setPowerMux1Callback);

void readSensor1Callback();
Task readSensor1Task(2000, TASK_ONCE, &readSensor1Callback);

void setPowerMux2Callback();
Task setPowerMux2Task(2000, TASK_ONCE, &setPowerMux2Callback);

void readSensor2Callback();
Task readSensor2Task(2000, TASK_ONCE, &readSensor2Callback);

void setPowerMux3Callback();
Task setPowerMux3Task(2000, TASK_ONCE, &setPowerMux3Callback);

void readSensor3Callback();
Task readSensor3Task(2000, TASK_ONCE, &readSensor3Callback);

void setPowerMux4Callback();
Task setPowerMux4Task(2000, TASK_ONCE, &setPowerMux4Callback);

void readSensor4Callback();
Task readSensor4Task(2000, TASK_ONCE, &readSensor4Callback);

// Create a master task
void masterTaskCallback();
Task masterTask(60000, TASK_FOREVER, &masterTaskCallback); // Run every 60000 ms (1 minute)

void setPowerMux1Callback() {
  Serial.printf("Setting power mux 1\n");
  // Set the power mux to the correct location
  sensorManager.set_pwr_mux(locationToPower[1]);
  // Enable the read sensor task
  readSensor1Task.set(2000, TASK_ONCE, &readSensor1Callback);
  readSensor1Task.enableDelayed(2000);

}

void readSensor1Callback() {
  StaticJsonDocument<200> sensorData;
  // Read the sensor data
  Serial.printf("Sensor 1 Type: %d\n", sensor1Type);
  Serial.printf("Sensor 2 Type: %d\n", sensor2Type);
  Serial.printf("Sensor 3 Type: %d\n", sensor3Type);
  Serial.printf("Sensor 4 Type: %d\n", sensor4Type);


  sensorData = sensorManager.readSensorData(sensorManager.returnSensorType(1), 1);
  // Serialize the data
  String jsonString;
  serializeJson(sensorData, jsonString);
  // Send the data
  // sendMessageHandler(jsonString, DATA, 1);
  Serial.printf("Sensor 1 data: %s\n", jsonString.c_str());

  // Enable the second power mux task
  setPowerMux2Task.set(8000, TASK_ONCE, &setPowerMux2Callback);
  setPowerMux2Task.enableDelayed(8000);
}

void setPowerMux2Callback() {
  // Set the power mux to the correct location
  sensorManager.set_pwr_mux(locationToPower[2]);
  // Enable the read sensor task
  readSensor2Task.set(2000, TASK_ONCE, &readSensor2Callback);
  readSensor2Task.enableDelayed(2000);
}

void readSensor2Callback() {
  StaticJsonDocument<200> sensorData;

  Serial.printf("Sensor 1 Type: %d\n", sensor1Type);
  Serial.printf("Sensor 2 Type: %d\n", sensor2Type);
  Serial.printf("Sensor 3 Type: %d\n", sensor3Type);
  Serial.printf("Sensor 4 Type: %d\n", sensor4Type);
  // Read the sensor data
  sensorData = sensorManager.readSensorData(sensorManager.returnSensorType(2), 2);
  // Serialize the data
  String jsonString;
  serializeJson(sensorData, jsonString);
  // Send the data
  // sendMessageHandler(jsonString, DATA, 2);
  Serial.printf("Sensor 2 data: %s\n", jsonString.c_str());

  // Enable the third power mux task
  setPowerMux3Task.set(8000, TASK_ONCE, &setPowerMux3Callback);
  setPowerMux3Task.enableDelayed(8000);
}

void setPowerMux3Callback() {
  // Set the power mux to the correct location
  sensorManager.set_pwr_mux(locationToPower[3]);
  // Enable the read sensor task
  readSensor3Task.set(2000, TASK_ONCE, &readSensor3Callback);
  readSensor3Task.enableDelayed(2000);
}

void readSensor3Callback() {
  StaticJsonDocument<200> sensorData;

  Serial.printf("Sensor 1 Type: %d\n", sensor1Type);
  Serial.printf("Sensor 2 Type: %d\n", sensor2Type);
  Serial.printf("Sensor 3 Type: %d\n", sensor3Type);
  Serial.printf("Sensor 4 Type: %d\n", sensor4Type);
  // Read the sensor data
  sensorData = sensorManager.readSensorData(sensorManager.returnSensorType(3), 3);
  // Serialize the data
  String jsonString;
  serializeJson(sensorData, jsonString);
  // Send the data
  // sendMessageHandler(jsonString, DATA, 3);
  Serial.printf("Sensor 3 data: %s\n", jsonString.c_str());

  // Enable the fourth power mux task
  setPowerMux4Task.set(8000, TASK_ONCE, &setPowerMux4Callback);
  setPowerMux4Task.enableDelayed(8000);
}

void setPowerMux4Callback() {
  // Set the power mux to the correct location
  sensorManager.set_pwr_mux(locationToPower[4]);
  // Enable the read sensor task
  readSensor4Task.set(2000, TASK_ONCE, &readSensor4Callback);
  readSensor4Task.enableDelayed(2000);
}

void readSensor4Callback() {
  StaticJsonDocument<200> sensorData;

  Serial.printf("Sensor 1 Type: %d\n", sensor1Type);
  Serial.printf("Sensor 2 Type: %d\n", sensor2Type);
  Serial.printf("Sensor 3 Type: %d\n", sensor3Type);
  Serial.printf("Sensor 4 Type: %d\n", sensor4Type);
  // Read the sensor data
  sensorData = sensorManager.readSensorData(sensorManager.returnSensorType(4), 4);
  // Serialize the data
  String jsonString;
  serializeJson(sensorData, jsonString);
  // Send the data
  // sendMessageHandler(jsonString, DATA, 4);
  Serial.printf("Sensor 4 data: %s\n", jsonString.c_str());
}

// The master task callback function
void masterTaskCallback() {
    // Enable the first power mux task
    Serial.printf("Master task running\n");
    setPowerMux1Task.set(2000, TASK_ONCE, &setPowerMux1Callback);
    setPowerMux1Task.enableDelayed(2000);
}








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
  userScheduler.execute();
}

void startSensorTask() {
  xTaskCreate(dataTask,      // Task function
              "DataTask",    // Task name
              10000,             // Stack size
              NULL,              // Task parameters
              1,                 // Task priority
              NULL);             // Task handle
  // userScheduler.addTask(dataTask);
  // dataTask.enable();

  // Start master task callback
  Serial.println("Starting master task\n");
  // userScheduler.addTask(masterTask);
  // userScheduler.addTask(setPowerMux1Task);
  // userScheduler.addTask(readSensor1Task);
  // userScheduler.addTask(setPowerMux2Task);
  // userScheduler.addTask(readSensor2Task);
  // userScheduler.addTask(setPowerMux3Task);
  // userScheduler.addTask(readSensor3Task);
  // userScheduler.addTask(setPowerMux4Task);
  // userScheduler.addTask(readSensor4Task);
  // masterTask.enable();
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
      Serial.printf("Sensor number: %d\n", sensorNumber);
      if(sensorManager.returnSensorType(sensorNumber) == UNKNOWN_SENSOR){
        Serial.println("Unknown sensor number\n");
        break;
      }


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

      // // Send the data message
      String jsonString;
      serializeJson(doc, jsonString);
      Serial.printf("Sending data message: %s\n", jsonString.c_str());
      mesh.sendSingle(to, jsonString);
      mesh.sendSingle(output, jsonString);

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
    if(!sensorSendFlags[0] && !sensorSendFlags[1] && !sensorSendFlags[2] && !sensorSendFlags[3] && !runningFlag) {
      // userScheduler.addTask(dataTask);
      // dataTask.enable();
      startSensorTask();
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
    eepromSensorType = returnSensorType(i);
    // Power the resistor lines
    sensorManager.set_pwr_mux(locationToResistor[i]);

    delay(1000);
    // Get the current sensor type
    sensorManager.getSingleSensorType(i);

    currentSensorType = sensorManager.returnSensorType(i);

    Serial.printf("Sensor %d type: %d\n", i, currentSensorType);
    // Check if the sensor type has been stored in EEPROM before
    if (returnChangeFlag(i)) {
      Serial.printf("Sensor %d has been stored in EEPROM before\n", i);
      // Read the sensor type from EEPROM
      // Check if the sensor type is the same as the one stored in EEPROM
      if(currentSensorType != eepromSensorType) {
        // Store the new sensor type in RAM and EEPROM
        setSingleSensorType(i, currentSensorType);
        
        // Send a hello message to update the sensor type on the backend
        Serial.printf("Sensor %d type has changed from %d to %d\n", i, eepromSensorType, currentSensorType);
        // Send a sensor hello if the type is not now unknown
        if(currentSensorType != UNKNOWN_SENSOR) {
          sendMessageHandler("", SENSORHELLO, i);
          Serial.printf("Sensor %d type has been sent to the backend\n", i);
          sensorSendFlags[i - 1] = 1;
        }
        else {
          Serial.printf("Sensor %d type is unknown\n", i);
          Serial.printf("No need to send a hello message\n");
          // Store the sensor type in EEPROM
          setSingleSensorType(i, currentSensorType);
        }
        // Send a hello message to update the sensor type on the backend
        // sendMessageHandler("", SENSORHELLO, i);
        // Serial.printf("Sensor %d type has been sent to the backend\n", i);
        // sensorSendFlags[i - 1] = 1;

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
      setSingleSensorType(i, currentSensorType);
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
  Serial.printf("Sensor send flags: %d, %d, %d, %d\n", sensorSendFlags[0], sensorSendFlags[1], sensorSendFlags[2], sensorSendFlags[3]);
  if(!sensorSendFlags[0] && !sensorSendFlags[1] && !sensorSendFlags[2] && !sensorSendFlags[3]) {
      // userScheduler.addTask(dataTask);
      // dataTask.enable();
      Serial.printf("Starting sensor task\n");
      Serial.printf("Sensor 1 type: %d\n", sensor1Type);
      Serial.printf("Sensor 2 type: %d\n", sensor2Type);
      Serial.printf("Sensor 3 type: %d\n", sensor3Type);
      Serial.printf("Sensor 4 type: %d\n", sensor4Type);
      startSensorTask();
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

// Task function to send data over mesh network
void dataTask(void *pvParameters) {
  (void)pvParameters; // Unused parameter
  runningFlag = 1;
  Serial.printf("Starting data task\n");
  for (;;) {
    StaticJsonDocument<200> sensorData;
    int lastSensorType;
    int currentSensorType;
    readSensorTypes();
    for(int i = 1; i < 5; i++) {
      
      // Record current sensor type as last sensor type
      lastSensorType = sensorManager.returnSensorType(i);
      // Get the single sensor type from the resistor value
      // Switch the power to the correct location
      sensorManager.set_pwr_mux(locationToResistor[i]);
      // Non-blocking delay for 1 seconds to allow power to switch
      vTaskDelay(pdMS_TO_TICKS(1000));
      sensorManager.getSingleSensorType(i);
      Serial.printf("Last sensor type: %d\n", lastSensorType);
      currentSensorType = sensorManager.returnSensorType(i);
      Serial.printf("Current sensor type: %d\n", currentSensorType);
      Serial.println();

      if(currentSensorType != lastSensorType) {
        Serial.printf("Changing sensor %d type from %d to %d\n", i, lastSensorType, currentSensorType);
        // Store the new sensor type in EEPROM
        setSingleSensorType(i, currentSensorType);
        
        if(currentSensorType == UNKNOWN_SENSOR) {
          Serial.printf("Sensor %d is unknown\n", i);
          Serial.printf("No need to send a hello message\n");
          // Store sensor type in EEPROM
        }
        else {
          // Send a hello message to update the sensor type on the backend
          sendMessageHandler("", SENSORHELLO, i);      
        }
        setSingleSensorType(i, currentSensorType);
        // Wait for the message to return
        vTaskDelay(pdMS_TO_TICKS(15000));
        // restart the loop
        // continue;    
      }

      // Check if data should be sent
      if(currentSensorType == UNKNOWN_SENSOR) {
        // Serial.printf("Sensor %d is unknown\n", i);
      }
      else {
        // Switch the power mux to the correct place
        sensorManager.set_pwr_mux(locationToPower[i]);
        // Non-blocking delay for 2 seconds to allow power to switch
        vTaskDelay(pdMS_TO_TICKS(2000));
        // Read the sensor data
        sensorData = sensorManager.readSensorData(currentSensorType, i);
        // Serialize the data
        String jsonString;
        serializeJson(sensorData, jsonString);
        // Send the data
        sendMessageHandler(jsonString, DATA, i);
        Serial.printf("Sensor %d data: %s\n", i, jsonString.c_str());
        Serial.printf("Sending data to the backend\n");
      }
    }
    readSensorTypes();
    // // Wait for 10 seconds before the next message
    vTaskDelay(pdMS_TO_TICKS(15000));
    // globalTaskCount++;
    // if(globalTaskCount > 4) {
    //   globalTaskCount = 1;
    // }
  }
}
