#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WiFiClient.h>
#include <EEPROM.h>

// Serial communication configuration
#define UART1_TX 5             // Pin 5
#define UART1_RX 4             // Pin 4
HardwareSerial SerialPort1(1); // Start serial port 1

// Define time server parameters
const char *ntpServer = "pool.ntp.org";
long gmtOffset_sec = 3600;
int daylightOffset_sec = 0;

// EEPROM configuration
#define EEPROM_SIZE 200
#define SSID_ADDRESS 0
#define PASSWORD_ADDRESS 100


// HTTP client configuration
// HTTPClient http;

// Function declarations
void parseSerial(void *pvParameters);
void printLocalTime();

// Setup
void setup() {
  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Allocate The Memory Size Needed
  EEPROM.begin(EEPROM_SIZE);

  // Start serial port for receiving data from provisioner
  SerialPort1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);

    // Connect to Wi-Fi
  WiFi.begin("Copleyhouse17", "chbos2017");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address 
  Serial.println(WiFi.localIP());

  // Find the timezone
  // String timezone_key = "8e7fe9b766244040b3946c6d264bc580";
  // String api_request = "https://api.ipgeolocation.io/ipgeo?apiKey=" + timezone_key + "&ip=" + WiFi.localIP().toString();
  // Serial.println(api_request);
  // http.begin(client, api_request);
  // int httpCode = http.GET();
  // if(httpCode > 0) {
  //   String payload = http.getString();
  //   Serial.printf("Payload: %s\n", payload.c_str());
  //   // Parse payload
  //   StaticJsonDocument<200> doc;
  //   DeserializationError error = deserializeJson(doc, payload);

  //   // Check for errors in parsing
  //   if (error) {
  //     Serial.print(F("deserializeJson() failed with code "));
  //     Serial.println(error.c_str());
  //     return;
  //   }

  //   // Extract timezone information
  //   gmtOffset_sec = doc["time_zone"]["offset"].as<long>() * 3600; // Convert to seconds
  //   // Initialize time
  //   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // }
  // http.end();




  // FreeRTOS task for handling serial port 1
  xTaskCreate(parseSerial,   // Function to run
              "SerialPort1Task", // Name of the task
              10000,             // Stack size (bytes)
              NULL,              // Parameter to pass
              1,                 // Task priority
              NULL               // Task handle
  );

}

// Loop (most code should be in tasks??)
void loop() {

}

// Function to parse serial data
void parseSerial(void *pvParameters) {
  for (;;) {
    if (SerialPort1.available()) {
      String input = SerialPort1.readStringUntil('\n');
      Serial.println(input);
    }
  }
}

// void printLocalTime()
// {
//   struct tm timeinfo;
//   if(!getLocalTime(&timeinfo)){
//     Serial.println("Failed to obtain time");
//     return;
//   }
//   Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
// }