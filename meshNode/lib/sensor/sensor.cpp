#include "sensor.h"
#include <stdexcept>

// DHT11 sensor initialization
DHTSensor::DHTSensor(SensorType type, int pin) : dht(pin, type) {
    dht.begin();
}

// DHT11 sensor class
void DHTSensor::readSensor() {
    // Read the sensor
    humidity = dht.readHumidity();
    temperature_c = dht.readTemperature();
    temperature_f = dht.readTemperature(true);
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temperature_c) || isnan(temperature_f)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
}

Sensor* DHTSensorFactory::createSensor(SensorType type, int pin) {
    switch (type) {
        case MESH_DHT11:
            return new DHTSensor(MESH_DHT11, pin);
        case MESH_DHT22:
            return new DHTSensor(MESH_DHT22, pin);
        default:
            // throw std::invalid_argument("Invalid sensor type");
            return nullptr;
    }
}