#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h> // Include the DHT library

// DHT11 sensor configuration
#define DHTPIN 20 // Define the pin for DHT11 data
#define DHTTYPE DHT11 // Define the type of DHT sensor
DHT dht(DHTPIN, DHTTYPE); // Create a DHT object

// Function declarations
int myFunction(int, int);
void dhtReadTask(void *pvParameters);

// Setup
void setup() {
  // Start serial monitoring for debugging
  Serial.begin(115200);

  // Start the DHT sensor
  dht.begin();

  // Create FreeRTOS task for reading from DT11
  xTaskCreate(dhtReadTask,      // Task function
              "DHTReadTask",    // Task name
              10000,             // Stack size
              NULL,              // Task parameters
              1,                 // Task priority
              NULL);             // Task handle
}

// Loop (most code should be in tasks)
void loop() {
  
}

// Task function to read DHT sensor data
void dhtReadTask(void *pvParameters) {
  (void)pvParameters; // Unused parameter

  for (;;) {
    // Read sensor values
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Print the values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%, Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");

    // Wait for 2 seconds before the next reading
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}