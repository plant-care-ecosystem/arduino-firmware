#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <cstring>
// Poster Cconfiguration
const char* ssid = "Copleyhouse17";
const char* password = "chbos2017";
const char* server = "plantcare-backend.azurewebsites.net";
WiFiClientSecure client;
String userID_temp = "admin";

// Posting locations
// Plants
String plant_post = "/plant";
String sensor_post = "/bensor";
String data_post = "/sensor/data";

// Serial communication configuration
#define UART1_TX 5             // Pin 5
#define UART1_RX 4             // Pin 4
HardwareSerial SerialPort1(1); // Start serial port 1

enum postType {
  POST,
  GET,
  PUT,
  UNKNOWN
};


// Function definitions
void wifiConnect();
void parseSerial(void *pvParameters);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // Start serial port for receiving data from provisioner
  SerialPort1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

  wifiConnect();

  // Create JSON document
  StaticJsonDocument<200> doc;
  doc["username"] = "admin";
  doc["password"] = "admin";

  String requestBody;
  serializeJson(doc, requestBody);

  client.setInsecure();

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    // client.println("GET /user/1 HTTP/1.1");
    // client.println("Host: plantcare-backend.azurewebsites.net");
    // client.println("Connection: close");
    // client.println();
    // Send HTTP POST request
    // client.println("POST /login HTTP/1.1");
    // client.println("Host: " + String(server));
    // client.println("Content-Type: application/json");
    // client.println("Content-Length: " + String(requestBody.length()));
    // client.println("Connection: close");
    // client.println();
    // client.println(requestBody);

      // Create the HTTP POST request
  // Create the HTTP POST request
  // String request = String("POST ") + String("/login") + String(" HTTP/1.1\r\n") +
  //                 String("Host: ") + String(server) + String("\r\n") +
  //                 String("Content-Type: application/json\r\n") +
  //                 String("Content-Length: ") + String(requestBody.length()) + String("\r\n") + String("\r\n") +
  //                 // String("Connection: close\r\n\r\n") +
  //                 String(requestBody);
  //   client.println(request);

  //   // Read HTTP response status line
  //   String statusLine = client.readStringUntil('\n');
  //   statusLine.trim();  // remove trailing whitespace

  //   // Check if POST was successful
  //   if (statusLine.startsWith("HTTP/1.1 200")) {
  //     Serial.println("POST successful");
  //   } else {
  //     Serial.println("POST failed");
  //     Serial.println("Response: " + statusLine);
  //   }
  // }

  // while (client.connected()) {
  //     String line = client.readStringUntil('\n');
  //     if (line == "\r") {
  //       Serial.println("headers received");
  //       break;
  //     }
  //   }
  //   // if there are incoming bytes available
  //   // from the server, read them and print them:
  //   while (client.available()) {
  //     char c = client.read();
  //     Serial.write(c);
     }

    // client.stop();
    // Serial.println("Closing connection.");

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

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }

  Serial.print("Connected to ");
  Serial.println(ssid);
}

uint32_t postHandler(String msg, postType httptype, String postLocation) {
  
  
  // Serial.println("\nStarting connection to server...");
  // if (!client.connect(server, 443))
  //   Serial.println("Connection failed!");
  // else {
  //   Serial.println("Connected to server!");
  // }

  switch (httptype)
    case PUT:
    {
      Serial.println();
      Serial.println("HTTPS Request String:");
      // Create request string
      String request = String("PUT ") + postLocation + String(" HTTP/1.1\r\n") +
                  String("Host: ") + String(server) + String("\r\n") +
                  String("Content-Type: application/json\r\n") +
                  String("Content-Length: ") + String(msg.length()) + String("\r\n") + String("\r\n") +
                  // String("Connection: close\r\n\r\n") +
                  msg;
      // Print request to serial monitor
      Serial.println(request);
      // Print request to server
      client.print(request);
      client.print("\r\n\r\n");
      // // Read HTTP response status line
      // // String statusLine = client.readStringUntil('\n');
      // // statusLine.trim();  // remove trailing whitespace
      // String statusLine = "";
      // while (client.available()) {
      //   statusLine += client.readStringUntil('\n');
      // }

      // // Check if POST was successful
      // if (statusLine.startsWith("HTTP/1.1 200")) {
      //   Serial.println("POST successful");
      //   Serial.println(statusLine);
      // } else {
      //   Serial.println("POST failed");
      //   Serial.println("Response: " + statusLine);
      // }

      // Read HTTP response status line
      String statusLine = client.readStringUntil('\n');
      statusLine.trim();  // remove trailing whitespace

      // Check if POST was successful
      if (statusLine.startsWith("HTTP/1.1 200")) {
        Serial.println("PUT successful");
      } else {
        Serial.println("PUT failed");
        Serial.println("Response: " + statusLine);
      }

      // // Read the rest of the response and store it in a variable
      // String response = "";
      // while (client.available()) {
      //   response += client.readStringUntil('\n');
      // }
      // Serial.println("Response: " + response);

      String response = "";
      bool headerFinished = false;
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (!headerFinished && line == "\r") {
          headerFinished = true;
        } else if (headerFinished) {
          response += line;
        }
      }
      response.trim(); // Remove any trailing whitespace
      Serial.println("Response: " + response);

      if (response.startsWith("[") && response.endsWith("]")) {
        response = response.substring(1, response.length() - 1);
      }

      StaticJsonDocument<200> doc;
      deserializeJson(doc, response);
      uint32_t dbId = doc["id"];
      Serial.print("DBID: ");
      Serial.println(dbId);

      return dbId;
    }

  // while (client.connected()) {
  //     String line = client.readStringUntil('\n');
  //     if (line == "\r") {
  //       Serial.println("headers received");
  //       break;
  //     }
  //   }
  //   // if there are incoming bytes available
  //   // from the server, read them and print them:
  //   while (client.available()) {
  //     char c = client.read();
  //     Serial.write(c);
  //   }

//     client.stop();
//     Serial.println("Closing connection.");
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
        uint32_t dbId = postHandler(jsonString, PUT, plant_post);

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
        uint32_t dbId = postHandler(jsonString, PUT, sensor_post);

        Serial.printf("Sensor ID: %u", dbId);

        StaticJsonDocument<200> returnDoc;
        returnDoc["type"] = "sensoradd";
        returnDoc["nodeId"] = doc["nodeId"];
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
        uint32_t dbId = postHandler(jsonString, PUT, data_post);

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
